#define CATCH_CONFIG_MAIN
#include "AlertSystem/alert_system.h"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include "adapter/MongoDBAdapter.hpp"
#include "alert/Alert.h"
#include "permission/permission.h"
#include "role/role.h"
#include "user/user.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <chrono>
#include <ctime>
#include <memory>
#include <mongocxx/instance.hpp>
#include <thread>
#include <vector>

// Set up a MongoDB instance for the test environment
mongocxx::instance instance{};

TEST_CASE("Alert System Integration Test") {
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

    // Declaration of neighbors vector to pass into Room below
    std::vector<Room *> neighbors;

    // Create Robot and Room instances using shared_ptr
    auto robot = std::make_shared<Robot>("CleaningRobot", 100);                   // Example attributes
    auto room = std::make_shared<Room>("MainRoom", 101, "wood", true, neighbors); // Example attributes

    // const std::string& roomName, int roomId, const std::string& flooringType, bool isRoomClean, std::vector<Room*>
    // neighbors

    // Create AlertSystem
    AlertSystem alertSystem;

    SECTION("Send and retrieve alerts") {
        dbAdapter.deleteAllAlerts();
        dbAdapter.deleteAllRobotStatuses();

        for (int i = 0; i < 6; ++i) {
            std::time_t currentTime = std::time(nullptr);
            std::string alertTitle = "Alert " + std::to_string(i + 1);
            std::string alertMessage = "Message for alert " + std::to_string(i + 1);

            // Create the alert using std::make_shared
            std::shared_ptr<Alert> alert = std::make_shared<Alert>(alertTitle, alertMessage, robot, room, currentTime);

            // Send alert to adminUser, updating the function call if necessary
            alertSystem.sendAlert(&adminUser, alert); // Ensure sendAlert can accept a shared_ptr

            // Save alert to MongoDB using the adapter (update parameter type if needed)
            dbAdapter.saveAlert(*alert); // If saveAlert takes an Alert by value or reference, use *alert

            // Update robot status and save asynchronously
            robot->depleteBattery(10);
            dbAdapter.saveRobotStatusAsync(robot);

            // Sleep for a short duration to simulate time between alerts
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));

        auto alerts = dbAdapter.retrieveAlerts();
        REQUIRE(alerts.size() == 6);

        for (const auto &alert : alerts) {
            REQUIRE_FALSE(alert.getTitle().empty());
            REQUIRE_FALSE(alert.getMessage().empty());
            alert.displayAlertInfo();
        }
    }

    SECTION("Delete and verify collections") {
        dbAdapter.deleteAllAlerts();
        dbAdapter.deleteAllRobotStatuses();

        auto alertsAfterDeletion = dbAdapter.retrieveAlerts();
        auto robotsAfterDeletion = dbAdapter.retrieveRobotStatuses();

        REQUIRE(alertsAfterDeletion.empty());
        REQUIRE(robotsAfterDeletion.empty());

        dbAdapter.dropAlertCollection();
        dbAdapter.dropRobotStatusCollection();
    }

    alertSystem.stop();
    dbAdapter.stop();
    dbAdapter.stopRobotStatusThread();
}
