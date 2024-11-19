#include <catch2/catch_test_macros.hpp>
#include "Schedular/Schedular.hpp"
#include "Robot/Robot.h"
#include "map/map.h"
#include <memory>

TEST_CASE("Scheduling System Test") {
    // Load test map
    Map map;
    REQUIRE_NOTHROW(map.loadFromFile("test_map.json"));

    std::vector<std::shared_ptr<Robot>> robots;
    
    // Create test robots
    auto robot1 = std::make_shared<Robot>("Robot1", 100);
    auto robot2 = std::make_shared<Robot>("Robot2", 100);

    // Set initial rooms for robots
    Room* room1 = map.getRoomById(1);
    Room* room2 = map.getRoomById(2);
    REQUIRE(room1 != nullptr);
    REQUIRE(room2 != nullptr);

    robot1->setCurrentRoom(room1);
    robot2->setCurrentRoom(room2);

    robots.push_back(robot1);
    robots.push_back(robot2);

    // Create scheduler with map and robots
    Scheduler scheduler(map, robots);

    SECTION("Robot Task Assignment") {
        // Test assigning tasks to existing rooms from test_map.json
        REQUIRE_NOTHROW(scheduler.assignCleaningTask("Robot1", 2, "Standard")); // Robot1 to Kitchen
        REQUIRE_NOTHROW(scheduler.assignCleaningTask("Robot2", 1, "Deep")); // Robot2 to Living Room

        // Test assigning task to non-existent robot
        REQUIRE_THROWS(scheduler.assignCleaningTask("NonexistentRobot", 1, "Standard"));

        // Test assigning task to non-existent room
        REQUIRE_THROWS(scheduler.assignCleaningTask("Robot1", 999, "Standard"));
    }
}
