#include "adapter/MongoDBAdapter.hpp"  // Corrected include path and case
#include "Robot/Robot.h"
#include "Room/Room.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

MongoDBAdapter::MongoDBAdapter(const std::string& uri, const std::string& dbName, const std::string& collectionName)
    : client_(mongocxx::uri{uri}) {
    auto db = client_[dbName];
    collection_ = db[collectionName];
}

void MongoDBAdapter::saveAlert(const Alert& alert) {
    // Convert Alert data to BSON document format
    auto alert_doc = make_document(
        kvp("title", alert.getTitle()),
        kvp("description", alert.getDescription()),
        kvp("robot_name", alert.getRobot()->getName()),
        kvp("room_name", alert.getRoom()->getRoomName()),
        kvp("timestamp", static_cast<int64_t>(alert.getTimestamp())) // Save as UNIX timestamp
    );

    // Insert document into MongoDB collection
    collection_.insert_one(alert_doc.view());
    std::cout << "Alert saved to MongoDB!" << std::endl;
}

std::vector<Alert> MongoDBAdapter::retrieveAlerts() {
    std::vector<Alert> alerts;

    auto cursor = collection_.find({});
    for (auto&& doc : cursor) {
        // Extract fields from BSON document
        std::string title = doc["title"].get_utf8().value.to_string();
        std::string description = doc["description"].get_utf8().value.to_string();
        std::string robot_name = doc["robot_name"].get_utf8().value.to_string();
        std::string room_name = doc["room_name"].get_utf8().value.to_string();
        int64_t timestamp = doc["timestamp"].get_int64().value;

        // Create a placeholder Robot and Room (assuming single Robot and Room instance per Alert)
        Robot robot(robot_name, 100);  // Example attributes
        Room room(room_name, 101);     // Example attributes

        // Create an Alert instance and add it to the vector
        Alert alert(title, description, &robot, &room, timestamp);
        alerts.push_back(alert);

        std::cout << "Retrieved Alert: " << bsoncxx::to_json(doc) << std::endl;
    }

    return alerts;
}
