// #define CATCH_CONFIG_MAIN
// #include <catch2/catch_test_macros.hpp>
// #include <catch2/matchers/catch_matchers_floating_point.hpp>
// #include <catch2/catch_approx.hpp>
// #include "alert_system/alert_system.h"
// #include "alert/Alert.h"
// #include "user/user.h"
// #include "Robot/Robot.h"
// #include "Room/Room.h"
// #include <memory>
// #include <ctime>
// #include <mongocxx/instance.hpp>

// mongocxx::instance instance{}; // Must be initialized before using the driver

// TEST_CASE("AlertSystem sends alerts and interacts with User, Robot, and Room", "[AlertSystem]") {
//     // Create an instance of AlertSystem
//     AlertSystem alertSystem;

//     // Create shared instances of Robot and Room
//     auto robot = std::make_shared<Robot>("TestRobot", 50);
//     auto room = std::make_shared<Room>("TestRoom", 101);

//     // Create User instances
//     User user1(1, "Alice", Role("UserRole"));
//     User user2(2, "Bob", Role("AdminRole"));

//     // Create Alert instances with various severities
//     std::time_t timestamp = std::time(nullptr);
//     auto alert1 = std::make_shared<Alert>("AlertType1", "Critical system failure!", robot, room, timestamp, Alert::Severity::HIGH);
//     auto alert2 = std::make_shared<Alert>("AlertType2", "Battery running low", robot, room, timestamp, Alert::Severity::MEDIUM);

//     // Send alerts to users
//     alertSystem.sendAlert(&user1, alert1);
//     alertSystem.sendAlert(&user2, alert2);

//     // Stop the AlertSystem to process the queue
//     alertSystem.stop();

//     // Check alert processing with receiveNotification (no MongoDB)
//     REQUIRE_NOTHROW(user1.receiveNotification(*alert1));
//     REQUIRE_NOTHROW(user2.receiveNotification(*alert2));

//     // Verify robot and room interactions
//     REQUIRE(robot->getName() == "TestRobot");
//     REQUIRE(robot->getBatteryLevel() == 50);  // Ensure initial battery level is set correctly
//     robot->depleteBattery(40);
//     REQUIRE(robot->getBatteryLevel() == 10);  // Verify battery depletion works

//     REQUIRE(room->getRoomName() == "TestRoom");
//     REQUIRE(room->getRoomId() == 101);  // Ensure room properties are correct

//     // Verify user details
//     REQUIRE(user1.getName() == "Alice");
//     REQUIRE(user2.getName() == "Bob");
// }

// test_AlertSystem.cpp

#include <catch2/catch_test_macros.hpp>  // Do NOT define CATCH_CONFIG_MAIN here
#include <mongocxx/instance.hpp>
#include "alert_system/alert_system.h"
#include "alert/Alert.h"
#include "user/user.h"
#include "role/role.h"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include <memory>
#include <exception>

// Create a global instance of mongocxx::instance
mongocxx::instance instance{}; // Must be initialized before using the driver

TEST_CASE("AlertSystem sends alerts and interacts with User, Robot, and Room") {
    try {
        // Create Roles and Users
        Role userRole("UserRole");
        User user1(1, "Alice", userRole);
        User user2(2, "Bob", userRole);

        // Create Robot and Room
        auto robot = std::make_shared<Robot>("TestRobot", 50);
        auto room = std::make_shared<Room>("TestRoom", 101);

        // Create Alerts
        std::time_t timestamp = std::time(nullptr);
        auto alert1 = std::make_shared<Alert>("AlertType1", "Critical system failure!", robot, room, timestamp, Alert::Severity::HIGH);
        auto alert2 = std::make_shared<Alert>("AlertType2", "Battery running low", robot, room, timestamp, Alert::Severity::MEDIUM);

        // Initialize AlertSystem
        AlertSystem alertSystem;

        // Send alerts
        alertSystem.sendAlert(&user1, alert1);
        alertSystem.sendAlert(&user2, alert2);

        // Stop the AlertSystem
        alertSystem.stop();

        // Verify user interactions
        REQUIRE(user1.getName() == "Alice");
        REQUIRE(user2.getName() == "Bob");

        // Verify robot and room details
        REQUIRE(robot->getName() == "TestRobot");
        REQUIRE(robot->getBatteryLevel() == 50);
        REQUIRE(room->getRoomName() == "TestRoom");
        REQUIRE(room->getRoomId() == 101);

        // Deplete robot battery and check
        robot->depleteBattery(40);
        REQUIRE(robot->getBatteryLevel() == 10);

    } catch (const std::exception& e) {
        FAIL("Exception caught: " << e.what());
    }
}
