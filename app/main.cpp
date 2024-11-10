#include "alert/Alert.h"
#include "alert_system/alert_system.h"
#include "user/user.h"
#include "role/role.h"
#include "permission/permission.h"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include <iostream>
#include <ctime>
#include "adapter/MongoDBAdapter.hpp"  // Include the adapter
#include <mongocxx/instance.hpp>       // For mongocxx::instance
#include <thread>                      // For std::this_thread::sleep_for
#include <chrono>                      // For std::chrono::seconds
#include <memory>                      // For std::shared_ptr

void testAlertSystem() {
    // The mongocxx::instance is initialized in main(), so we remove it here.

    // Initialize MongoDB Adapter
    std::string uri = "mongodb://localhost:27017";
    std::string dbName = "mydb";
    MongoDBAdapter dbAdapter(uri, dbName);

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

    // Create Robot and Room instances using shared_ptr
    auto robot = std::make_shared<Robot>("CleaningRobot", 100);  // Example attributes
    auto room = std::make_shared<Room>("MainRoom", 101);         // Example attributes

    // Create AlertSystem
    AlertSystem alertSystem;

    // Delete existing alerts and robot statuses before starting
    dbAdapter.deleteAllAlerts();
    dbAdapter.deleteAllRobotStatuses();

    // Simulate sending multiple alerts and saving to MongoDB
    for (int i = 0; i < 6; ++i) {
        // Create an alert
        std::time_t currentTime = std::time(nullptr);
        std::string alertTitle = "Alert " + std::to_string(i + 1);
        std::string alertMessage = "Message for alert " + std::to_string(i + 1);

        // Use std::make_shared to create a shared_ptr<Alert>
        auto alert = std::make_shared<Alert>(alertTitle, alertMessage, robot, room, currentTime);

        // Send alert to adminUser
        alertSystem.sendAlert(&adminUser, alert);

        // Save alert to MongoDB using the adapter
        dbAdapter.saveAlert(*alert);

        // Update robot status and save asynchronously
        robot->depleteBattery(10);  // Decrease battery level by 10%
        dbAdapter.saveRobotStatusAsync(robot);

        // Sleep for a short duration to simulate time between alerts
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Give some time for all alerts and robot statuses to be processed
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Retrieve all alerts from MongoDB using the adapter
    auto alerts = dbAdapter.retrieveAlerts();

    // Display the retrieved alerts
    for (const auto& alert : alerts) {
        alert.displayAlertInfo();
    }

    // Retrieve all robot statuses from MongoDB using the adapter
    auto robots = dbAdapter.retrieveRobotStatuses();

    // Display the retrieved robot statuses
    for (const auto& r : robots) {
        std::cout << "Robot Name: " << r->getName()
                  << ", Battery Level: " << r->getBatteryLevel() << "%" << std::endl;
    }

    // Test delete methods
    dbAdapter.deleteAllAlerts();
    dbAdapter.deleteAllRobotStatuses();

    // Verify that collections are empty
    auto alertsAfterDeletion = dbAdapter.retrieveAlerts();
    auto robotsAfterDeletion = dbAdapter.retrieveRobotStatuses();

    std::cout << "\nAlerts after deletion: " << alertsAfterDeletion.size() << std::endl;
    std::cout << "Robot statuses after deletion: " << robotsAfterDeletion.size() << std::endl;

    // Optionally, drop the collections
    dbAdapter.dropAlertCollection();
    dbAdapter.dropRobotStatusCollection();

    // Stop the alert system and database adapter threads before exiting
    alertSystem.stop();
    dbAdapter.stop();
    dbAdapter.stopRobotStatusThread();
}

int main() {
    // Initialize MongoDB driver instance
    mongocxx::instance instance{};

    // Run the test function
    testAlertSystem();

    return 0;
}
