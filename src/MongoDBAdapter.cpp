// MongoDBAdapter.cpp

#include "adapter/MongoDBAdapter.hpp"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/exception.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>
#include <memory>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

// Constructor
MongoDBAdapter::MongoDBAdapter(const std::string& uri, const std::string& dbName)
    : dbName_(dbName), client_(mongocxx::uri{uri}), db_(client_[dbName]), running_(true) {
    
    // Clear existing collections on startup
    dropAlertCollection();
    dropRobotStatusCollection();
    
    std::cout << "MongoDB adapter initialized. Database cleared." << std::endl;
    
    // Start background threads
    robotStatusThread_ = std::thread(&MongoDBAdapter::processRobotStatusQueue, this);
    alertThread_ = std::thread(&MongoDBAdapter::processAlertQueue, this);
}

// Destructor
MongoDBAdapter::~MongoDBAdapter() {
    // Clear all data before shutting down
    deleteAllAlerts();
    deleteAllRobotStatuses();
    stop();
}

// Save alert synchronously
void MongoDBAdapter::saveAlert(const Alert& alert) {
    auto alertCollection = db_["alerts"];

    // Convert Alert to BSON and save to MongoDB
    auto alert_doc = make_document(
        kvp("title", alert.getTitle()),
        kvp("description", alert.getDescription()),
        kvp("robot_name", alert.getRobot()->getName()),
        kvp("room_name", alert.getRoom()->getRoomName()),
        kvp("timestamp", static_cast<int64_t>(alert.getTimestamp()))
    );

    try {
        alertCollection.insert_one(alert_doc.view());
        std::cout << "Alert saved to MongoDB: " << alert.getTitle() << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error inserting alert into MongoDB: " << e.what() << std::endl;
    }
}

// Save robot status synchronously
void MongoDBAdapter::saveRobotStatus(std::shared_ptr<Robot> robot) {
    if (!robot) {
        std::cerr << "Attempted to save null robot status" << std::endl;
        return;
    }

    auto robotCollection = db_["robot_status"];

    try {
        // First, delete any existing status for this robot
        deleteRobotStatus(robot->getName());

        // Create BSON document with robot status
        auto status_doc = make_document(
            kvp("name", robot->getName()),
            kvp("battery_level", robot->getBatteryLevel()),
            kvp("water_level", robot->getWaterLevel()),
            kvp("status", robot->getStatus()),
            kvp("current_room", robot->getCurrentRoom() ? robot->getCurrentRoom()->getRoomName() : "Unknown"),
            kvp("movement_progress", robot->getMovementProgress()),
            kvp("is_cleaning", robot->isCleaning()),
            kvp("is_charging", robot->isCharging()),
            kvp("needs_maintenance", robot->needsMaintenance()),
            kvp("low_battery_alert_sent", robot->isLowBatteryAlertSent()),
            kvp("low_water_alert_sent", robot->isLowWaterAlertSent())
        );

        robotCollection.insert_one(status_doc.view());
        std::cout << "Robot status saved to MongoDB: " << robot->getName() << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error saving robot status to MongoDB: " << e.what() << std::endl;
    }
}

// Delete robot status by name
void MongoDBAdapter::deleteRobotStatus(const std::string& robotName) {
    auto robotCollection = db_["robot_status"];
    try {
        robotCollection.delete_one(make_document(kvp("name", robotName)));
        std::cout << "Robot status deleted from MongoDB: " << robotName << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error deleting robot status from MongoDB: " << e.what() << std::endl;
    }
}

// Delete all robot statuses
void MongoDBAdapter::deleteAllRobotStatuses() {
    auto robotCollection = db_["robot_status"];
    try {
        robotCollection.delete_many({});
        std::cout << "All robot statuses deleted from MongoDB" << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error deleting all robot statuses from MongoDB: " << e.what() << std::endl;
    }
}

// Drop robot status collection
void MongoDBAdapter::dropRobotStatusCollection() {
    try {
        db_["robot_status"].drop();
        std::cout << "Robot status collection dropped from MongoDB" << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error dropping robot status collection from MongoDB: " << e.what() << std::endl;
    }
}

// Drop alert collection
void MongoDBAdapter::dropAlertCollection() {
    try {
        db_["alerts"].drop();
        std::cout << "Alert collection dropped from MongoDB" << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error dropping alert collection from MongoDB: " << e.what() << std::endl;
    }
}

// Stop all background threads
void MongoDBAdapter::stop() {
    if (!running_) return;
    
    running_ = false;
    cv_.notify_all();
    
    if (robotStatusThread_.joinable()) {
        robotStatusThread_.join();
    }
    if (alertThread_.joinable()) {
        alertThread_.join();
    }
    
    std::cout << "MongoDB adapter stopped" << std::endl;
}

// Process robot status queue
void MongoDBAdapter::processRobotStatusQueue() {
    std::queue<std::shared_ptr<Robot>> localQueue;
    
    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (robotStatusQueue_.empty()) {
            cv_.wait_for(lock, std::chrono::seconds(1));
            continue;
        }
        
        // Process all queued statuses
        std::swap(localQueue, robotStatusQueue_);
        lock.unlock();
        
        while (!localQueue.empty()) {
            auto robot = localQueue.front();
            localQueue.pop();
            if (robot) {
                saveRobotStatus(robot);
            }
        }
    }
}

// Save robot status asynchronously
void MongoDBAdapter::saveRobotStatusAsync(std::shared_ptr<Robot> robot) {
    if (!robot) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    robotStatusQueue_.push(robot);
    cv_.notify_one();
}

// Save alert asynchronously
void MongoDBAdapter::saveAlertAsync(const Alert& alert) {
    if (!running_) return;
    std::lock_guard<std::mutex> lock(mutex_);
    alertQueue_.push(std::make_shared<Alert>(alert));
    cv_.notify_one();
}

// Process alert queue
void MongoDBAdapter::processAlertQueue() {
    std::queue<std::shared_ptr<Alert>> localQueue;
    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (alertQueue_.empty()) {
            cv_.wait_for(lock, std::chrono::seconds(1));
            continue;
        }
        std::swap(localQueue, alertQueue_);
        lock.unlock();
        
        while (!localQueue.empty()) {
            auto alert = localQueue.front();
            localQueue.pop();
            if (alert) {
                saveAlert(*alert);
            }
        }
    }
}

// Retrieve alerts
std::vector<Alert> MongoDBAdapter::retrieveAlerts() {
    std::vector<Alert> alerts;

    // Create client instance locally
    auto alertCollection = db_["alerts"];

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

// Delete all alerts
void MongoDBAdapter::deleteAllAlerts() {
    // Create client instance locally
    auto alertCollection = db_["alerts"];

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

// Retrieve robot statuses
std::vector<std::shared_ptr<Robot>> MongoDBAdapter::retrieveRobotStatuses() {
    std::vector<std::shared_ptr<Robot>> robots;

    // Create client instance locally
    auto robotStatusCollection = db_["robot_status"];

    auto cursor = robotStatusCollection.find({});
    for (auto&& doc : cursor) {
        // Extract fields from BSON document
        std::string name = doc["name"].get_string().value.to_string();
        int battery_level = doc["battery_level"].get_int32().value;
        int water_level = doc["water_level"].get_int32().value; // Extract water level
        // Extract other attributes as needed

        // Create a Robot instance and add it to the vector
        auto robot = std::make_shared<Robot>(name, battery_level, water_level); // Pass water level to Robot constructor
        robots.push_back(robot);

        std::cout << "Retrieved Robot: " << bsoncxx::to_json(doc) << std::endl;
    }

    return robots;
}

// Stop robot status monitoring thread
void MongoDBAdapter::stopRobotStatusThread() {
    stop(); // For now, this is the same as stop()
}