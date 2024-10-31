// #include "AlertSystem/alert_system.h"
// #include "user/user.h"
// #include "role/role.h"
// #include "permission/permission.h"
// #include "Robot/Robot.h"
// #include "Room/Room.h"
// #include <iostream>
// #include <ctime>

// // Function to simulate sending alerts
// void testAlertSystem() {
//     // Create Permissions
//     Permission adminPermission("ADMIN");
//     Permission userPermission("USER");

//     // Create Roles
//     Role adminRole("Admin");
//     adminRole.addPermission(adminPermission);

//     Role userRole("User");
//     userRole.addPermission(userPermission);

//     // Create Users
//     User adminUser(1, "AdminUser", adminRole);
//     User regularUser(2, "RegularUser", userRole);

//     // Create Robot and Room instances
//     Robot robot("CleaningRobot", 100);  // Example attributes
//     Room room("MainRoom", 101);         // Example attributes

//     // Create an alert
//     std::time_t currentTime = std::time(nullptr);
//     Alert cleaningAlert("Cleaning", "Robot needs maintenance", &robot, &room, currentTime);

//     // Create AlertSystem
//     AlertSystem alertSystem;

//     // Simulate sending alerts
//     std::cout << "Testing sending alert to AdminUser:" << std::endl;
//     alertSystem.sendAlert(&adminUser, &cleaningAlert);


//     std::cout << "Testing the robot status:" << std::endl;
//     robot.sendStatusUpdate();

//     std::cout << "\nTesting sending alert to RegularUser:" << std::endl;
//     alertSystem.sendAlert(&regularUser, &cleaningAlert);
// }

// int main() {
//     // Run the test function
//     testAlertSystem();
//     return 0;
// }

#include "alert/Alert.h"
#include "AlertSystem/alert_system.h"
#include "user/user.h"
#include "role/role.h"
#include "permission/permission.h"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include <iostream>
#include <ctime>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

void saveAlertToDB(const Alert& alert, mongocxx::collection& collection) {
    // Convert Alert data to BSON document format
    auto alert_doc = make_document(
        kvp("title", alert.getTitle()),
        kvp("description", alert.getDescription()),
        kvp("robot_name", alert.getRobot()->getName()),
        kvp("room_name", alert.getRoom()->getName()),
        kvp("timestamp", static_cast<int64_t>(alert.getTimestamp())) // Save as UNIX timestamp
    );

    // Insert document into MongoDB collection
    collection.insert_one(alert_doc.view());
    std::cout << "Alert saved to MongoDB!" << std::endl;
}

void retrieveAlertsFromDB(mongocxx::collection& collection) {
    std::cout << "Retrieving all alerts from MongoDB:" << std::endl;

    // Query all documents in the collection
    auto cursor = collection.find({});
    for (auto&& doc : cursor) {
        std::cout << bsoncxx::to_json(doc) << std::endl;
    }
}

// Function to simulate sending alerts and saving to MongoDB
void testAlertSystem() {
    // Initialize MongoDB
    mongocxx::instance instance{};
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);

    auto db = client["mydb"];
    auto collection = db["alerts"];

    // Create Permissions
    Permission adminPermission("ADMIN");
    Permission userPermission("USER");

    // Create Roles
    Role adminRole("Admin");
    adminRole.addPermission(adminPermission);

    Role userRole("User");
    userRole.addPermission(userPermission);

    // Create Users
    User adminUser(1, "AdminUser", adminRole);
    User regularUser(2, "RegularUser", userRole);

    // Create Robot and Room instances
    Robot robot("CleaningRobot", 100);  // Example attributes
    Room room("MainRoom", 101);         // Example attributes

    // Create an alert
    std::time_t currentTime = std::time(nullptr);
    Alert cleaningAlert("Cleaning", "Robot needs maintenance", &robot, &room, currentTime);

    // Create AlertSystem
    AlertSystem alertSystem;

    // Simulate sending alerts and saving to MongoDB
    std::cout << "Testing sending alert to AdminUser:" << std::endl;
    alertSystem.sendAlert(&adminUser, &cleaningAlert);
    saveAlertToDB(cleaningAlert, collection);  // Save alert to MongoDB

    std::cout << "Testing the robot status:" << std::endl;
    robot.sendStatusUpdate();

    std::cout << "\nTesting sending alert to RegularUser:" << std::endl;
    alertSystem.sendAlert(&regularUser, &cleaningAlert);

    // Retrieve all alerts from MongoDB
    retrieveAlertsFromDB(collection);
}

int main() {
    // Run the test function
    testAlertSystem();
    return 0;
}
