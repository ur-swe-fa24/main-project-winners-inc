// MongoDBAdapter.cpp

#include "adapter/MongoDBAdapter.hpp"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/exception.hpp>
#include <iostream>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

// Constructor
MongoDBAdapter::MongoDBAdapter(const std::string& uri, const std::string& dbName)
    : dbName_(dbName) {
    // Initialization if needed
}

// Destructor
MongoDBAdapter::~MongoDBAdapter() {
    // Cleanup if needed
}

// Save alert synchronously
void MongoDBAdapter::saveAlert(const Alert& alert) {
    // Create client instance locally
    mongocxx::client client(mongocxx::uri{});
    auto db = client[dbName_];
    auto alertCollection = db["alerts"];

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
        std::cout << "Alert saved to MongoDB!" << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error inserting alert into MongoDB: " << e.what() << std::endl;
    }
}

// Retrieve alerts
std::vector<Alert> MongoDBAdapter::retrieveAlerts() {
    std::vector<Alert> alerts;

    // Create client instance locally
    mongocxx::client client(mongocxx::uri{});
    auto db = client[dbName_];
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

// Delete all alerts
void MongoDBAdapter::deleteAllAlerts() {
    // Create client instance locally
    mongocxx::client client(mongocxx::uri{});
    auto db = client[dbName_];
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
    // Create client instance locally
    mongocxx::client client(mongocxx::uri{});
    auto db = client[dbName_];
    auto alertCollection = db["alerts"];

    try {
        alertCollection.drop();
        std::cout << "Alert collection dropped successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error dropping alert collection: " << e.what() << std::endl;
    }
}

// Save robot status synchronously
void MongoDBAdapter::saveRobotStatus(std::shared_ptr<Robot> robot) {
    // Create client instance locally
    mongocxx::client client(mongocxx::uri{});
    auto db = client[dbName_];
    auto robotStatusCollection = db["robot_statuses"];

    // Convert Robot data to BSON document format
    auto robot_doc = make_document(
        kvp("name", robot->getName()),
        kvp("battery_level", robot->getBatteryLevel())
        // Add other robot attributes as needed
    );

    try {
        robotStatusCollection.insert_one(robot_doc.view());
        std::cout << "Robot status saved to MongoDB!" << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error inserting robot status into MongoDB: " << e.what() << std::endl;
    }
}

// Retrieve robot statuses
std::vector<std::shared_ptr<Robot>> MongoDBAdapter::retrieveRobotStatuses() {
    std::vector<std::shared_ptr<Robot>> robots;

    // Create client instance locally
    mongocxx::client client(mongocxx::uri{});
    auto db = client[dbName_];
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

// Delete all robot statuses
void MongoDBAdapter::deleteAllRobotStatuses() {
    // Create client instance locally
    mongocxx::client client(mongocxx::uri{});
    auto db = client[dbName_];
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
    // Create client instance locally
    mongocxx::client client(mongocxx::uri{});
    auto db = client[dbName_];
    auto robotStatusCollection = db["robot_statuses"];

    try {
        robotStatusCollection.drop();
        std::cout << "Robot status collection dropped successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error dropping robot status collection: " << e.what() << std::endl;
    }
}
