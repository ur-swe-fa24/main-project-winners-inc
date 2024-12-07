#include <catch2/catch_test_macros.hpp>
#include "Schedular/Schedular.hpp"
#include "config/ResourceConfig.hpp"
#include "Robot/Robot.h"
#include "map/map.h"
#include <memory>
#include <filesystem>

TEST_CASE("Test Scheduling System", "[scheduling]") {
    // Initialize resource config with the correct path
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path resourcePath = currentPath / "resources";
    if (!std::filesystem::exists(resourcePath)) {
        resourcePath = currentPath / ".." / "resources";
    }
    config::ResourceConfig::initialize(resourcePath.string());
    
    Map map;
    REQUIRE_NOTHROW(map.loadFromFile(config::ResourceConfig::getMapPath()));

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
    Scheduler scheduler(&map, &robots);

    SECTION("Robot Task Assignment") {
        // Test assigning tasks to existing rooms from test_map.json
        // Note: Due to virtual wall between rooms 3 and 5, we expect this to throw
        REQUIRE_THROWS_AS(scheduler.assignCleaningTask("Robot1", 5, "Standard"), 
                         std::runtime_error);

        // Test assigning task to robot's current room (should work)
        REQUIRE_NOTHROW(scheduler.assignCleaningTask("Robot1", 3, "Vacuum")); // Robot1 to its current room
        REQUIRE_NOTHROW(scheduler.assignCleaningTask("Robot2", 5, "Scrub")); // Robot2 to its current room

        // Test assigning task to non-existent robot
        REQUIRE_THROWS(scheduler.assignCleaningTask("NonexistentRobot", 3, "Vacuum"));

        // Test assigning task to non-existent room
        REQUIRE_THROWS(scheduler.assignCleaningTask("Robot1", 999, "Vacuum"));
    }
}
