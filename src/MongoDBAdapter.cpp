#include "adapter/MongoDBAdapter.hpp"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <mongocxx/exception/exception.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

// Constructor
MongoDBAdapter::MongoDBAdapter(const std::string& uri, const std::string& dbName)
    : client_(mongocxx::uri{uri}), dbName_(dbName), running_(true), robotStatusRunning_(true) {

    // Start the database threads
    dbThread_ = std::thread(&MongoDBAdapter::processSaveQueue, this);
    robotStatusThread_ = std::thread(&MongoDBAdapter::processRobotStatusQueue, this);
}

// Destructor
MongoDBAdapter::~MongoDBAdapter() {
    // Ensure the database threads stop and resources are cleaned up
    stop();
    stopRobotStatusThread();
}

// Save alert asynchronously
void MongoDBAdapter::saveAlert(const Alert& alert) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        saveQueue_.push(alert);
    }
    cv_.notify_one();
}

// Process save queue for alerts
void MongoDBAdapter::processSaveQueue() {
    // Create collection instance in this thread
    auto db = client_[dbName_];
    auto alertCollection = db["alerts"];

    while (running_) {
        std::unique_lock<std::mutex> lock(queueMutex_);
        cv_.wait(lock, [this]() { return !saveQueue_.empty() || !running_; });

        while (!saveQueue_.empty()) {
            Alert alert = saveQueue_.front();
            saveQueue_.pop();
            lock.unlock();

            // Convert Alert to BSON and save to MongoDB
            auto alert_doc = make_document(
                kvp("title", alert.getTitle()),
                kvp("description", alert.getDescription()),
                kvp("robot_name", alert.getRobot()->getName()),
                kvp("room_name", alert.getRoom()->getRoomName()),
                kvp("timestamp", static_cast<int64_t>(alert.getTimestamp()))
            );

            // Insert using local collection instance
            try {
                alertCollection.insert_one(alert_doc.view());
                std::cout << "Alert saved to MongoDB!" << std::endl;
            } catch (const mongocxx::exception& e) {
                std::cerr << "Error inserting alert into MongoDB: " << e.what() << std::endl;
            }

            lock.lock();
        }
    }
}

// Retrieve alerts
std::vector<Alert> MongoDBAdapter::retrieveAlerts() {
    std::vector<Alert> alerts;

    auto db = client_[dbName_];
    auto alertCollection = db["alerts"];

    auto cursor = alertCollection.find({});
    for (auto&& doc : cursor) {
        // Extract fields from BSON document
        std::string title = doc["title"].get_string().value.to_string();
        std::string description = doc["description"].get_string().value.to_string();
        std::string robot_name = doc["robot_name"].get_string().value.to_string();
        std::string room_name = doc["room_name"].get_string().value.to_string();
        int64_t timestamp = doc["timestamp"].get_int64().value;

        // Create shared_ptr instances of Robot and Room
        auto robot = std::make_shared<Robot>(robot_name, 100);  // Example attributes
        auto room = std::make_shared<Room>(room_name, 101);     // Example attributes

        // Create an Alert instance and add it to the vector
        Alert alert(title, description, robot, room, timestamp);
        alerts.push_back(alert);

        std::cout << "Retrieved Alert: " << bsoncxx::to_json(doc) << std::endl;
    }

    return alerts;
}

// Stop alert processing thread
void MongoDBAdapter::stop() {
    running_ = false;
    cv_.notify_all();
    if (dbThread_.joinable()) {
        dbThread_.join();
    }
}

// Delete all alerts
void MongoDBAdapter::deleteAllAlerts() {
    auto db = client_[dbName_];
    auto alertCollection = db["alerts"];

    bsoncxx::document::value empty_filter = make_document();
    try {
        auto result = alertCollection.delete_many(empty_filter.view());
        if (result) {
            std::cout << "Deleted " << result->deleted_count() << " alerts from MongoDB." << std::endl;
        } else {
            std::cout << "No alerts were deleted from MongoDB." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error deleting alerts: " << e.what() << std::endl;
    }
}

// Drop alert collection
void MongoDBAdapter::dropAlertCollection() {
    auto db = client_[dbName_];
    auto alertCollection = db["alerts"];

    try {
        alertCollection.drop();
        std::cout << "Alert collection dropped successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error dropping alert collection: " << e.what() << std::endl;
    }
}

// Save robot status asynchronously
void MongoDBAdapter::saveRobotStatusAsync(std::shared_ptr<Robot> robot) {
    {
        std::lock_guard<std::mutex> lock(robotStatusMutex_);
        robotStatusQueue_.push(robot);
    }
    robotStatusCV_.notify_one();
}

// Process robot status queue
void MongoDBAdapter::processRobotStatusQueue() {
    // Create collection instance in this thread
    auto db = client_[dbName_];
    auto robotStatusCollection = db["robot_statuses"];

    while (robotStatusRunning_) {
        std::unique_lock<std::mutex> lock(robotStatusMutex_);
        robotStatusCV_.wait(lock, [this]() { return !robotStatusQueue_.empty() || !robotStatusRunning_; });

        while (!robotStatusQueue_.empty()) {
            auto robot = robotStatusQueue_.front();
            robotStatusQueue_.pop();
            lock.unlock();

            // Convert Robot data to BSON document format
            auto robot_doc = make_document(
                kvp("name", robot->getName()),
                kvp("battery_level", robot->getBatteryLevel())
                // Add other robot attributes as needed
            );

            // Insert using local collection instance
            try {
                robotStatusCollection.insert_one(robot_doc.view());
                std::cout << "Robot status saved to MongoDB!" << std::endl;
            } catch (const mongocxx::exception& e) {
                std::cerr << "Error inserting robot status into MongoDB: " << e.what() << std::endl;
            }

            lock.lock();
        }
    }
}

// Retrieve robot statuses
std::vector<std::shared_ptr<Robot>> MongoDBAdapter::retrieveRobotStatuses() {
    std::vector<std::shared_ptr<Robot>> robots;

    auto db = client_[dbName_];
    auto robotStatusCollection = db["robot_statuses"];

    auto cursor = robotStatusCollection.find({});
    for (auto&& doc : cursor) {
        // Extract fields from BSON document
        std::string name = doc["name"].get_string().value.to_string();
        int battery_level = doc["battery_level"].get_int32().value;
        // Extract other attributes as needed

        // Create a Robot instance and add it to the vector
        auto robot = std::make_shared<Robot>(name, battery_level);
        robots.push_back(robot);

        std::cout << "Retrieved Robot: " << bsoncxx::to_json(doc) << std::endl;
    }

    return robots;
}

// Stop robot status processing thread
void MongoDBAdapter::stopRobotStatusThread() {
    robotStatusRunning_ = false;
    robotStatusCV_.notify_all();
    if (robotStatusThread_.joinable()) {
        robotStatusThread_.join();
    }
}

// Delete all robot statuses
void MongoDBAdapter::deleteAllRobotStatuses() {
    auto db = client_[dbName_];
    auto robotStatusCollection = db["robot_statuses"];

    bsoncxx::document::value empty_filter = make_document();
    try {
        auto result = robotStatusCollection.delete_many(empty_filter.view());
        if (result) {
            std::cout << "Deleted " << result->deleted_count() << " robot statuses from MongoDB." << std::endl;
        } else {
            std::cout << "No robot statuses were deleted from MongoDB." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error deleting robot statuses: " << e.what() << std::endl;
    }
}

// Drop robot status collection
void MongoDBAdapter::dropRobotStatusCollection() {
    auto db = client_[dbName_];
    auto robotStatusCollection = db["robot_statuses"];

    try {
        robotStatusCollection.drop();
        std::cout << "Robot status collection dropped successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error dropping robot status collection: " << e.what() << std::endl;
    }
}
