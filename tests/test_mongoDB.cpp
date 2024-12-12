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

// Create a global instance of mongocxx::instance
mongocxx::instance instance{}; // Must be initialized before using the driver

TEST_CASE("MongoDB Integration Test") {
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
    auto robot = std::make_shared<Robot>("CleaningBot", 100.0, Robot::Size::MEDIUM, Robot::Strategy::VACUUM, 50.0);  // Example attributes
    auto room = std::make_shared<Room>("MainRoom", 101, "wood", "medium", true, neighbors);  // Example attributes

    // Create AlertSystem
    AlertSystem alertSystem;

    SECTION("Basic MongoDB Operations") {
        // Clear any existing data
        dbAdapter.deleteAllAlerts();
        dbAdapter.deleteAllRobotStatuses();

        // Test saving and retrieving a single alert
        std::time_t currentTime = std::time(nullptr);
        std::string alertTitle = "Test Alert";
        std::string alertMessage = "Test Message";

        auto alert = std::make_shared<Alert>(alertTitle, alertMessage, robot, room, currentTime);
        dbAdapter.saveAlert(*alert);

        auto alerts = dbAdapter.retrieveAlerts();
        REQUIRE(alerts.size() == 1);
        REQUIRE(alerts[0].getTitle() == alertTitle);
        REQUIRE(alerts[0].getMessage() == alertMessage);
    }

    SECTION("Robot Status Operations") {
        // Clear existing data
        dbAdapter.deleteAllRobotStatuses();

        // Save robot status
        dbAdapter.saveRobotStatus(robot);
        
        // Wait for async operations to complete
        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto robots = dbAdapter.retrieveRobotStatuses();
        REQUIRE(robots.size() == 1);
        REQUIRE(robots[0]->getName() == robot->getName());
        REQUIRE(robots[0]->getBatteryLevel() == robot->getBatteryLevel());
    }

    SECTION("Async Operations") {
        dbAdapter.deleteAllAlerts();
        dbAdapter.deleteAllRobotStatuses();

        // Test async alert saving
        std::time_t currentTime = std::time(nullptr);
        auto alert = std::make_shared<Alert>("Async Alert", "Test Async", robot, room, currentTime);
        dbAdapter.saveAlertAsync(*alert);

        // Test async robot status saving
        dbAdapter.saveRobotStatusAsync(robot);

        // Wait for async operations to complete
        std::this_thread::sleep_for(std::chrono::seconds(2));

        auto alerts = dbAdapter.retrieveAlerts();
        auto robots = dbAdapter.retrieveRobotStatuses();

        REQUIRE(alerts.size() == 1);
        REQUIRE(robots.size() == 1);
    }

    // Stop all threads before exiting
    dbAdapter.stop();
    dbAdapter.stopRobotStatusThread();
}
