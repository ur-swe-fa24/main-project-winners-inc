// // #define CATCH_CONFIG_MAIN
// #include <catch2/catch_test_macros.hpp>
// #include <catch2/matchers/catch_matchers_floating_point.hpp>
// #include <catch2/catch_approx.hpp>
// #include "alert/Alert.h"
// #include "alert_system/alert_system.h"
// #include "user/user.h"
// #include "role/role.h"
// #include "permission/permission.h"
// #include "Robot/Robot.h"
// #include "Room/Room.h"
// #include "adapter/MongoDBAdapter.hpp"
// #include <mongocxx/instance.hpp>
// #include <memory>
// #include <thread>
// #include <chrono>
// #include <ctime>

// // Set up a MongoDB instance for the test environment
// mongocxx::instance instance{};

// TEST_CASE("Alert System Integration Test") {
//     // The mongocxx::instance is initialized in main(), so we remove it here.

//     // Initialize MongoDB Adapter
//     std::string uri = "mongodb://localhost:27017";
//     std::string dbName = "mydb";
//     MongoDBAdapter dbAdapter(uri, dbName);

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

//     // Create Robot and Room instances using shared_ptr
//     auto robot = std::make_shared<Robot>("CleaningRobot", 100);  // Example attributes
//     auto room = std::make_shared<Room>("MainRoom", 101);         // Example attributes

//     // Create AlertSystem
//     AlertSystem alertSystem;

//     SECTION("Send and retrieve alerts") {
//         dbAdapter.deleteAllAlerts();
//         dbAdapter.deleteAllRobotStatuses();

//         for (int i = 0; i < 6; ++i) {
//             std::time_t currentTime = std::time(nullptr);
//             std::string alertTitle = "Alert " + std::to_string(i + 1);
//             std::string alertMessage = "Message for alert " + std::to_string(i + 1);

//             // Create the alert using std::make_shared
//             std::shared_ptr<Alert> alert = std::make_shared<Alert>(alertTitle, alertMessage, robot, room, currentTime);

//             // Send alert to adminUser, updating the function call if necessary
//             alertSystem.sendAlert(&adminUser, alert);  // Ensure sendAlert can accept a shared_ptr

//             // Save alert to MongoDB using the adapter (update parameter type if needed)
//             dbAdapter.saveAlert(*alert);  // If saveAlert takes an Alert by value or reference, use *alert

//             // Update robot status and save asynchronously
//             robot->depleteBattery(10);
//             dbAdapter.saveRobotStatusAsync(robot);

//             // Sleep for a short duration to simulate time between alerts
//             std::this_thread::sleep_for(std::chrono::milliseconds(500));
//         }



//         std::this_thread::sleep_for(std::chrono::seconds(2));

//         auto alerts = dbAdapter.retrieveAlerts();
//         REQUIRE(alerts.size() == 6);

//         for (const auto& alert : alerts) {
//             REQUIRE_FALSE(alert.getTitle().empty());
//             REQUIRE_FALSE(alert.getMessage().empty());
//             alert.displayAlertInfo();
//         }
//     }

//     SECTION("Delete and verify collections") {
//         dbAdapter.deleteAllAlerts();
//         dbAdapter.deleteAllRobotStatuses();

//         auto alertsAfterDeletion = dbAdapter.retrieveAlerts();
//         auto robotsAfterDeletion = dbAdapter.retrieveRobotStatuses();

//         REQUIRE(alertsAfterDeletion.empty());
//         REQUIRE(robotsAfterDeletion.empty());

//         dbAdapter.dropAlertCollection();
//         dbAdapter.dropRobotStatusCollection();
//     }

//     alertSystem.stop();
//     dbAdapter.stop();
//     dbAdapter.stopRobotStatusThread();
// }

// test_mongoDB.cpp

#define CATCH_CONFIG_MAIN  // Only in one test file

#include <catch2/catch_test_macros.hpp>
#include <mongocxx/instance.hpp>
#include "adapter/MongoDBAdapter.hpp"
#include "Robot/Robot.h"
#include "alert/Alert.h"
#include <memory>
#include <exception>

// Create a global instance of mongocxx::instance
mongocxx::instance instance{}; // Must be initialized before using the driver

TEST_CASE("MongoDB Adapter Test") {
    try {
        // Initialize MongoDB Adapter
        std::string uri = "mongodb://localhost:27017";
        std::string dbName = "testdb";
        auto dbAdapter = std::make_shared<MongoDBAdapter>(uri, dbName);

        // Clean up collections
        dbAdapter->deleteAllAlerts();
        dbAdapter->deleteAllRobotStatuses();

        // Test saving a robot
        auto robot = std::make_shared<Robot>("TestBot", 100);
        dbAdapter->saveRobotStatus(robot);

        // Retrieve robots
        auto retrievedRobots = dbAdapter->retrieveRobotStatuses();
        REQUIRE(!retrievedRobots.empty());
        REQUIRE(retrievedRobots[0]->getName() == "TestBot");

        // Test saving an alert
        auto room = std::make_shared<Room>("TestRoom", 1);
        Alert alert("Test Alert", "This is a test alert", robot, room, std::time(nullptr));
        dbAdapter->saveAlert(alert);

        // Retrieve alerts
        auto alerts = dbAdapter->retrieveAlerts();
        REQUIRE(!alerts.empty());
        REQUIRE(alerts[0].getTitle() == "Test Alert");
    } catch (const std::exception& e) {
        FAIL("Exception caught: " << e.what());
    }
}
