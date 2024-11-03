#include "alert/Alert.h"
#include "AlertSystem/alert_system.h"
#include "user/user.h"
#include "role/role.h"
#include "permission/permission.h"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include <iostream>
#include <ctime>
#include "adapter/MongoDBAdapter.hpp"  // Include the adapter

// Add this include statement for mongocxx::instance
#include <mongocxx/instance.hpp>       // **Include this header**

void testAlertSystem() {
    // Initialize MongoDB (instance should be created once)
    static mongocxx::instance instance{};

    // Initialize MongoDB Adapter
    std::string uri = "mongodb://localhost:27017";
    std::string dbName = "mydb";
    std::string collectionName = "alerts";
    MongoDBAdapter dbAdapter(uri, dbName, collectionName);

    // Rest of your code...
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
    dbAdapter.saveAlert(cleaningAlert);  // Use the adapter to save alert to MongoDB

    std::cout << "Testing the robot status:" << std::endl;
    robot.sendStatusUpdate();

    std::cout << "\nTesting sending alert to RegularUser:" << std::endl;
    alertSystem.sendAlert(&regularUser, &cleaningAlert);

    // Retrieve all alerts from MongoDB using the adapter
    auto alerts = dbAdapter.retrieveAlerts();

    // Optionally, display the retrieved alerts
    for (const auto& alert : alerts) {
        alert.displayAlertInfo();
    }
}

int main() {
    // Run the test function
    testAlertSystem();
    return 0;
}
