#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_approx.hpp>
#include "alert/Alert.h"
#include "AlertSystem/alert_system.h"
#include "user/user.h"
#include "role/role.h"
#include "permission/permission.h"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include "adapter/MongoDBAdapter.hpp"
#include <mongocxx/instance.hpp>
#include <memory>
#include <thread>
#include <chrono>
#include <ctime>
#include <vector>

// Create a global instance of mongocxx::instance
mongocxx::instance instance{}; // Must be initialized before using the driver

TEST_CASE("Alert System Integration Test") {
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
    User adminUser("AdminUser", std::make_shared<Role>(adminRole));
    User regularUser("RegularUser", std::make_shared<Role>(userRole));

    // Declaration of neighbors vector
    std::vector<Room*> neighbors;

    // Create Robot and Room instances using shared_ptr
    auto robot = std::make_shared<Robot>("CleaningRobot", 100);  // Example attributes
    auto room = std::make_shared<Room>("MainRoom", 101, "wood", true, neighbors);  // Example attributes

    // Create AlertSystem
    AlertSystem alertSystem;

    SECTION("Send and retrieve alerts") {
        dbAdapter.deleteAllAlerts();
        dbAdapter.deleteAllRobotStatuses();

        for (int i = 0; i < 6; ++i) {
            std::time_t currentTime = std::time(nullptr);
            std::string alertType = "TEST_ALERT";
            std::string alertMessage = "Test alert message " + std::to_string(i + 1);

            // Create the alert using std::make_shared
            std::shared_ptr<Alert> alert = std::make_shared<Alert>(alertType, alertMessage, robot, room, currentTime);

            // Send alert to adminUser
            alertSystem.sendAlert(&adminUser, alert);

            // Save alert to MongoDB
            dbAdapter.saveAlert(*alert);

            // Update robot status and save asynchronously
            robot->depleteBattery(10);
            dbAdapter.saveRobotStatusAsync(robot);

            // Sleep for a short duration to simulate time between alerts
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));

        auto alerts = dbAdapter.retrieveAlerts();
        REQUIRE(alerts.size() == 6);

        for (const auto& alert : alerts) {
            REQUIRE_FALSE(alert.getType().empty());
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

    // Cleanup
    dbAdapter.stop();
}