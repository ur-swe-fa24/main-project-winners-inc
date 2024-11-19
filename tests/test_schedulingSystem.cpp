#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_approx.hpp>
#include <memory>
#include "schedular/Schedular.hpp"
#include "map/map.h"
#include "robot/Robot.h"
#include "room/Room.h"

TEST_CASE("Scheduler assigns and executes cleaning tasks") {
    // Create real Map, Robot, and Room instances
    Map map;  // Assuming Map has real implementations
    auto robot1 = std::make_shared<Robot>("CleaningRobot1", map.getRoomById(101)); // Starts at LivingRoom
    std::vector<std::shared_ptr<Robot>> robots = {robot1};
    Scheduler scheduler(map, robots);

    SECTION("Assign cleaning task to robot") {
        // Assign a task to clean the Kitchen (room 102) with strategy "FastClean"
        scheduler.assignCleaningTask("CleaningRobot1", 102, "FastClean");

        // Check if the robot's target room is set correctly
        REQUIRE(robot1->getCurrentRoom()->getRoomId() == 102);
    }

    SECTION("Assign cleaning task to non-existent robot") {
        // Try assigning a task to a non-existent robot
        scheduler.assignCleaningTask("NonExistentRobot", 102, "FastClean");

        // The system should print an error message, but for testing, we can check for side effects
        // In a real test, you might mock std::cerr or check logs
    }

    SECTION("Execute cleaning task") {
        // Assign a cleaning task
        scheduler.assignCleaningTask("CleaningRobot1", 102, "FastClean");

        // Execute the cleaning task
        scheduler.executeCleaning(robot1, map.getRoomById(102), "FastClean");

        // Check if the robot's battery is depleted after cleaning (depends on the cleaning time for "Tile")
        REQUIRE(robot1->getBatteryLevel() != 0); // Battery should be reduced, but not zero
        REQUIRE(map.getRoomById(102)->isRoomClean == true); // Kitchen should be marked clean
    }

    SECTION("Cleaning with battery depletion") {
        // Assign task and deplete robot's battery
        robot1->depleteBattery(95);  // Deplete almost all battery
        scheduler.assignCleaningTask("CleaningRobot1", 102, "FastClean");
        scheduler.executeCleaning(robot1, map.getRoomById(102), "FastClean");

        // The robot's battery should be depleted after executing the cleaning task
        REQUIRE(robot1->getBatteryLevel() == 0); // Battery should be zero or below
    }
}
