#include <catch2/catch_test_macros.hpp>
#include "Schedular/Schedular.hpp"
#include "Robot/Robot.h"
#include "map/map.h"
#include <memory>

TEST_CASE("Scheduling System Test") {
    // Load test map
    Map map;
    REQUIRE_NOTHROW(map.loadFromFile("map.json"));

    std::vector<std::shared_ptr<Robot>> robots;
    
    // Create test robots
    auto robot1 = std::make_shared<Robot>("Robot1", 100);
    auto robot2 = std::make_shared<Robot>("Robot2", 100);

    // Set initial rooms for robots
    Room* room3 = map.getRoomById(3);
    Room* room5 = map.getRoomById(5);
    REQUIRE(room3 != nullptr);
    REQUIRE(room5 != nullptr);

    robot1->setCurrentRoom(room3);
    robot2->setCurrentRoom(room5);

    robots.push_back(robot1);
    robots.push_back(robot2);

    // Create scheduler with map and robots
    Scheduler scheduler(map, robots);

    SECTION("Robot Task Assignment") {
        // Test assigning tasks to existing rooms from test_map.json
        // Note: Due to virtual wall between rooms 3 and 5, we expect this to throw
        REQUIRE_THROWS_AS(scheduler.assignCleaningTask("Robot1", 5, "Standard"), 
                         std::runtime_error);

        // Test assigning task to robot's current room (should work)
        REQUIRE_NOTHROW(scheduler.assignCleaningTask("Robot1", 3, "Standard")); // Robot1 to its current room
        REQUIRE_NOTHROW(scheduler.assignCleaningTask("Robot2", 5, "Deep")); // Robot2 to its current room

        // Test assigning task to non-existent robot
        REQUIRE_THROWS(scheduler.assignCleaningTask("NonexistentRobot", 3, "Standard"));

        // Test assigning task to non-existent room
        REQUIRE_THROWS(scheduler.assignCleaningTask("Robot1", 999, "Standard"));
    }
}
