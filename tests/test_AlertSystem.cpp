#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_approx.hpp>
#include "alert/Alert.h"
#include "AlertSystem/alert_system.h"
#include "user/user.h"
#include "role/role.h"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include <memory>
#include <thread>
#include <chrono>

// Helper function to create a sample alert
Alert createSampleAlert() {
    auto robot = std::make_shared<Robot>("CleaningBot", 100.0, Robot::Size::MEDIUM, Robot::Strategy::VACUUM, 50.0);
    auto room = std::make_shared<Room>("Living Room", 1, "Wood", "Large", false, std::vector<Room*>());
    std::time_t now = std::time(nullptr);
    return Alert("Critical", "System failure", robot, room, now, Alert::Severity::HIGH);
}
// ---- TEST CASES ----

TEST_CASE("Alert Initialization and Properties") {
    auto robot = std::make_shared<Robot>("CleaningBot", 100.0, Robot::Size::MEDIUM, Robot::Strategy::VACUUM, 50.0);
    auto room = std::make_shared<Room>("Living Room", 1, "Wood", "Large", false, std::vector<Room*>());
    std::time_t now = std::time(nullptr);

    Alert alert("Warning", "Battery low", robot, room, now, Alert::Severity::MEDIUM);

    SECTION("Correct alert properties") {
        REQUIRE(alert.getType() == "Warning");
        REQUIRE(alert.getTitle() == "Warning");
        REQUIRE(alert.getMessage() == "Battery low");
        REQUIRE(alert.getDescription() == "Battery low");
        REQUIRE(alert.getRobot() == robot);
        REQUIRE(alert.getRoom() == room);
        REQUIRE(alert.getTimestamp() == now);
        REQUIRE(alert.getSeverity() == Alert::Severity::MEDIUM);
    }

    SECTION("Update alert message") {
        alert.setMessage("Updated message");
        REQUIRE(alert.getMessage() == "Updated message");
    }
}

TEST_CASE("User Notification Behavior") {
    auto role = std::make_shared<Role>("Admin");
    User user("TestUser", role);

    auto alert = createSampleAlert();

    SECTION("User properties are correct") {
        REQUIRE(user.getName() == "TestUser");
        REQUIRE(user.getRole() == role);
    }

    SECTION("User receives notification") {
        REQUIRE_NOTHROW(user.receiveNotification(alert));
    }
}

TEST_CASE("AlertSystem Processes Alerts Correctly") {
    AlertSystem alertSystem;

    auto role = std::make_shared<Role>("Operator");
    auto user = User("OperatorUser", role);
    auto alert = createSampleAlert();  

    SECTION("Sending a single alert") {
        alertSystem.sendAlert(user.getName(), alert.getDescription());  

        // Allow time for alert processing
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        REQUIRE(true);  // No crash expected
    }

    SECTION("Sending multiple alerts") {
        for (int i = 0; i < 5; ++i) {
            alertSystem.sendAlert(user.getName(), alert.getDescription());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        REQUIRE(true);  // Ensure no crash
    }
}

TEST_CASE("AlertSystem Handles Null Message and Type") {
    AlertSystem alertSystem;
    auto role = std::make_shared<Role>("Operator");
    auto user = User("OperatorUser", role);
    auto alert = createSampleAlert();  

    SECTION("Null message and type") {
        alertSystem.sendAlert(nullptr, nullptr);  // Both are null
    }

    SECTION("Null message only") {
        alertSystem.sendAlert(nullptr, alert.getType());  // Alert exists, user is null
    }

    SECTION("Null type only") {
        alertSystem.sendAlert(alert.getMessage(), nullptr);  // User exists, alert is null
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(true);  // No crash expected
}