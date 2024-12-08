#include "adapter/MongoDBAdapter.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/exception.hpp>
#include <iostream>

// Using declarations
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

// Constructor
MongoDBAdapter::MongoDBAdapter(const std::string& uri, const std::string& dbName)
    : dbName_(dbName), client_(mongocxx::uri{uri}), db_(client_[dbName]), running_(true) {

    // Clear existing collections on startup
    dropAlertCollection();
    dropRobotStatusCollection();
    dropRoomsCollection(); 
    std::cout << "MongoDB adapter initialized. Database cleared." << std::endl;
    
    // Start background threads
    robotStatusThread_ = std::thread(&MongoDBAdapter::processRobotStatusQueue, this);
    alertThread_ = std::thread(&MongoDBAdapter::processAlertQueue, this);
    roomThread_ = std::thread(&MongoDBAdapter::processRoomQueue, this); // Start room processing thread
}

// Destructor
MongoDBAdapter::~MongoDBAdapter() {
    // Clean up all threads
    stop();
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
    if (roomThread_.joinable()) { // Join the room processing thread
        roomThread_.join();
    }
    
    std::cout << "MongoDB adapter stopped" << std::endl;
}

// Stop robot status monitoring thread
void MongoDBAdapter::stopRobotStatusThread() {
    if (!running_) return;
    
    running_ = false;
    cv_.notify_all();
    
    if (robotStatusThread_.joinable()) {
        robotStatusThread_.join();
    }
    
    std::cout << "Robot status monitoring thread stopped" << std::endl;
}

// Alert methods implementation
void MongoDBAdapter::saveAlert(const Alert& alert) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto alertCollection = db_["alerts"];

    // Convert Alert to BSON and save to MongoDB
    auto alert_doc = make_document(
        kvp("title", alert.getTitle()),
        kvp("description", alert.getDescription()),
        kvp("robot_name", alert.getRobot() ? alert.getRobot()->getName() : "None"),
        kvp("room_name", alert.getRoom() ? alert.getRoom()->getRoomName() : "None"),
        kvp("timestamp", static_cast<int64_t>(alert.getTimestamp())),
        kvp("severity", static_cast<int32_t>(alert.getSeverity()))
    );
    try {
        alertCollection.insert_one(alert_doc.view());
        std::cout << "Alert saved to MongoDB: " << alert.getTitle() << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error inserting alert into MongoDB: " << e.what() << std::endl;
    }
}

void MongoDBAdapter::saveAlertAsync(const Alert& alert) {
    if (!running_) {
        std::cout << "saveAlertAsync called after adapter stopped" << std::endl;
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto clonedAlert = std::make_shared<Alert>(alert);
    std::cout << "saveAlertAsync: Pushing alert into queue" << std::endl;
    alertQueue_.push(clonedAlert);
    cv_.notify_one();
}

std::vector<Alert> MongoDBAdapter::retrieveAlerts() {
    std::vector<Alert> alerts;
    auto alertCollection = db_["alerts"];

    try {
        auto cursor = alertCollection.find({});
        for (auto&& doc : cursor) {
            std::string title = doc["title"].get_string().value.to_string();
            std::string description = doc["description"].get_string().value.to_string();
            std::string robot_name = doc["robot_name"].get_string().value.to_string();
            std::string room_name = doc["room_name"].get_string().value.to_string();
            int64_t timestamp = doc["timestamp"].get_int64().value;
            int severityInt = doc["severity"].get_int32().value;
            Alert::Severity severity = static_cast<Alert::Severity>(severityInt);

            // Create shared_ptr instances of Robot and Room
            auto robot = std::make_shared<Robot>(robot_name, 100.0, Robot::Size::MEDIUM, Robot::Strategy::VACUUM, 100.0);
            auto room = std::make_shared<Room>(room_name, 101);     // Example attributes

            // Create an Alert instance and add it to the vector
            Alert alert(title, description, robot, room, timestamp, severity);
            alerts.push_back(alert);

            std::cout << "Retrieved Alert: " << bsoncxx::to_json(doc) << std::endl;
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error retrieving alerts from MongoDB: " << e.what() << std::endl;
    }

    return alerts;
}

void MongoDBAdapter::deleteAllAlerts() {
    auto alertCollection = db_["alerts"];
    try {
        auto result = alertCollection.delete_many({});
        if (result) {
            std::cout << "Deleted " << result->deleted_count() << " alerts from MongoDB." << std::endl;
        } else {
            std::cout << "No alerts were deleted from MongoDB." << std::endl;
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error deleting alerts: " << e.what() << std::endl;
    }
}

void MongoDBAdapter::dropAlertCollection() {
    try {
        db_["alerts"].drop();
        std::cout << "Alert collection dropped from MongoDB" << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error dropping alert collection from MongoDB: " << e.what() << std::endl;
    }
}

// In MongoDBAdapter.cpp

void MongoDBAdapter::dropRoomsCollection() {
    try {
        db_["rooms"].drop();
        std::cout << "Rooms collection dropped from MongoDB" << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error dropping rooms collection from MongoDB: " << e.what() << std::endl;
    }
}


// Robot status methods implementation
void MongoDBAdapter::saveRobotStatus(std::shared_ptr<Robot> robot) {
    if (!robot) {
        std::cerr << "Attempted to save null robot status" << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto robotCollection = db_["robot_status"];

    try {
        // First, delete any existing status for this robot
        robotCollection.delete_one(make_document(kvp("name", robot->getName())));

        // Create BSON document with robot status
        auto status_doc = make_document(
            kvp("name", robot->getName()),
            kvp("battery_level", static_cast<double>(robot->getBatteryLevel())),
            kvp("water_level", static_cast<double>(robot->getWaterLevel())),
            kvp("status", robot->getStatus()),
            kvp("current_room", robot->getCurrentRoom() ? robot->getCurrentRoom()->getRoomName() : "Unknown"),
            kvp("movement_progress", static_cast<double>(robot->getMovementProgress())),
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

void MongoDBAdapter::saveRobotStatusAsync(std::shared_ptr<Robot> robot) {
    if (!robot) return;
    std::lock_guard<std::mutex> lock(mutex_);
    robotStatusQueue_.push(robot);
    cv_.notify_one();
}

std::vector<std::shared_ptr<Robot>> MongoDBAdapter::retrieveRobotStatuses() {
    std::vector<std::shared_ptr<Robot>> robots;

    auto robotStatusCollection = db_["robot_status"];

    try {
        auto cursor = robotStatusCollection.find({});
        for (auto&& doc : cursor) {
            std::string name = doc["name"].get_string().value.to_string();
            double battery_level = doc["battery_level"].get_double().value;
            double water_level = doc["water_level"].get_double().value;
            std::string status = doc["status"].get_string().value.to_string();
            std::string current_room = doc["current_room"].get_string().value.to_string();
            double movement_progress = doc["movement_progress"].get_double().value;
            bool is_cleaning = doc["is_cleaning"].get_bool().value;
            bool is_charging = doc["is_charging"].get_bool().value;
            bool needs_maintenance = doc["needs_maintenance"].get_bool().value;
            bool low_battery_alert_sent = doc["low_battery_alert_sent"].get_bool().value;
            bool low_water_alert_sent = doc["low_water_alert_sent"].get_bool().value;

            // Create a Robot instance and add it to the vector
            auto robot = std::make_shared<Robot>(name, battery_level, Robot::Size::MEDIUM, Robot::Strategy::VACUUM, water_level);
            // Set additional attributes as needed
            // robot->setStatus(status); // Implement setters if necessary
            // robot->setCurrentRoom(current_room); // Implement setters if necessary

            robots.push_back(robot);

            std::cout << "Retrieved Robot: " << bsoncxx::to_json(doc) << std::endl;
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error retrieving robot statuses from MongoDB: " << e.what() << std::endl;
    }

    return robots;
}

void MongoDBAdapter::deleteRobotStatus(const std::string& robotName) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto robotCollection = db_["robot_status"];
    try {
        robotCollection.delete_one(make_document(kvp("name", robotName)));
        std::cout << "Robot status deleted from MongoDB: " << robotName << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error deleting robot status from MongoDB: " << e.what() << std::endl;
    }
}

void MongoDBAdapter::deleteAllRobotStatuses() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto robotCollection = db_["robot_status"];
    try {
        robotCollection.delete_many({});
        std::cout << "All robot statuses deleted from MongoDB" << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error deleting all robot statuses from MongoDB: " << e.what() << std::endl;
    }
}

void MongoDBAdapter::dropRobotStatusCollection() {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        db_["robot_status"].drop();
        std::cout << "Robot status collection dropped from MongoDB" << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error dropping robot status collection from MongoDB: " << e.what() << std::endl;
    }
}

// Room methods implementation
void MongoDBAdapter::saveRoomStatus(const Room& room) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto roomsCollection = db_["rooms"];
    
    // Create BSON document
    auto room_doc = make_document(
        kvp("roomId", room.getRoomId()),
        kvp("roomName", room.getRoomName()),
        kvp("flooringType", room.getFlooringType()),
        kvp("size", room.getSize()),
        kvp("isRoomClean", room.isRoomClean)
    );
    
    try {
        // Upsert the room document based on roomId
        roomsCollection.replace_one(
            make_document(kvp("roomId", room.getRoomId())),
            room_doc.view(),
            mongocxx::options::replace{}.upsert(true)
        );
        std::cout << "Room status saved to MongoDB: " << room.getRoomName() << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error saving room status to MongoDB: " << e.what() << std::endl;
    }
}

void MongoDBAdapter::saveRoomStatusAsync(const Room& room) {
    if (!running_) return;
    std::lock_guard<std::mutex> lock(mutex_);
    auto clonedRoom = std::make_shared<Room>(*const_cast<Room*>(&room));
    roomQueue_.push(clonedRoom);
    cv_.notify_one();
}

// In MongoDBAdapter.cpp

void MongoDBAdapter::loadRoomStatuses(std::vector<Room*>& rooms) {
    auto roomsCollection = db_["rooms"];

    try {
        auto cursor = roomsCollection.find({});
        for (auto&& doc : cursor) {
            // Check if "roomId" exists
            if (doc.find("roomId") == doc.end()) {
                std::cerr << "Document missing 'roomId' field: " << bsoncxx::to_json(doc) << std::endl;
                continue;
            }
            int roomId = doc["roomId"].get_int32().value;

            // Similarly check for "isRoomClean"
            if (doc.find("isRoomClean") == doc.end()) {
                std::cerr << "Document missing 'isRoomClean' field: " << bsoncxx::to_json(doc) << std::endl;
                continue;
            }
            bool isRoomClean = doc["isRoomClean"].get_bool().value;

            // Update room status
            for (auto& room : rooms) {
                if (room->getRoomId() == roomId) {
                    room->isRoomClean = isRoomClean;
                    std::cout << "Room " << room->getRoomName() << " loaded as "
                              << (isRoomClean ? "clean" : "dirty") << " from database." << std::endl;
                    break;
                }
            }
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error loading room statuses from MongoDB: " << e.what() << std::endl;
    }
}

void MongoDBAdapter::initializeRooms(const std::vector<Room*>& rooms) {
    auto roomsCollection = db_["rooms"];
    
    try {
        // Check if rooms collection is empty
        auto count = roomsCollection.count_documents({});
        if (count == 0) {
            // Insert all rooms
            for (const auto& room : rooms) {
                auto room_doc = make_document(
                    kvp("roomId", room->getRoomId()),
                    kvp("roomName", room->getRoomName()),
                    kvp("flooringType", room->getFlooringType()),
                    kvp("size", room->getSize()),
                    kvp("isRoomClean", room->isRoomClean)
                );
                roomsCollection.insert_one(room_doc.view());
                std::cout << "Room inserted into MongoDB: " << room->getRoomName() << std::endl;
            }
        } else {
            // Rooms already exist, load clean statuses
            loadRoomStatuses(const_cast<std::vector<Room*>&>(rooms));
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error initializing rooms in MongoDB: " << e.what() << std::endl;
    }
}

// Process room queue for asynchronous operations
void MongoDBAdapter::processRoomQueue() {
    std::queue<std::shared_ptr<Room>> localQueue;
    
    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !roomQueue_.empty() || !running_; });
        
        if (!running_) break;
        
        std::swap(localQueue, roomQueue_);
        lock.unlock();
        
        while (!localQueue.empty()) {
            auto room = localQueue.front();
            localQueue.pop();
            if (room) {
                saveRoomStatus(*room);
            }
        }
    }
}

void MongoDBAdapter::processAlertQueue() {
    std::queue<std::shared_ptr<Alert>> localQueue; // Only define once outside the loop

    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !alertQueue_.empty() || !running_; });

        if (!running_) break;

        // Do not redefine localQueue here
        std::swap(localQueue, alertQueue_);
        lock.unlock();

        while (!localQueue.empty()) {
            auto alert = localQueue.front();
            localQueue.pop();
            if (alert) {
                std::cout << "processAlertQueue: Saving alert..." << std::endl;
                saveAlert(*alert);
            }
        }
    }
}

void MongoDBAdapter::processRobotStatusQueue() {
    std::queue<std::shared_ptr<Robot>> localQueue;
    
    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !robotStatusQueue_.empty() || !running_; });
        
        if (!running_) break;
        
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


void MongoDBAdapter::saveRobotAnalytics(std::shared_ptr<Robot> robot) {
    if (!robot) return;
    std::lock_guard<std::mutex> lock(mutex_);
    auto analyticsCollection = db_["robot_analytics"];

    // Upsert robot analytics based on robot name
    auto filter = make_document(kvp("name", robot->getName()));
    auto update = make_document(
        kvp("$set", make_document(
            kvp("error_count", robot->getErrorCount()),
            kvp("total_work_time", robot->getTotalWorkTime())
        ))
    );

    analyticsCollection.update_one(filter.view(), update.view(), mongocxx::options::update{}.upsert(true));
}

std::vector<std::tuple<std::string,int,double>> MongoDBAdapter::retrieveRobotAnalytics() {
    std::vector<std::tuple<std::string,int,double>> data;
    auto analyticsCollection = db_["robot_analytics"];
    try {
        auto cursor = analyticsCollection.find({});
        for (auto&& doc : cursor) {
            std::string name = doc["name"].get_string().value.to_string();
            int error_count = doc["error_count"].get_int32();
            double total_work_time = doc["total_work_time"].get_double().value;
            data.push_back(std::make_tuple(name, error_count, total_work_time));
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error retrieving robot analytics from MongoDB: " << e.what() << std::endl;
    }
    return data;
}
