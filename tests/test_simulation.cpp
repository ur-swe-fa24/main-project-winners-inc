#include <catch2/catch_test_macros.hpp>
#include "RobotSimulator/RobotSimulator.hpp"
#include "adapter/MongoDBAdapter.hpp"
#include "config/ResourceConfig.hpp"
#include "Robot/Robot.h"
#include "Scheduler/Scheduler.hpp"
#include "AlertSystem/alert_system.h"
#include "map/map.h"
#include <memory>
#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <iostream>
#include <filesystem>

// Create a global instance of mongocxx::instance
mongocxx::instance instance{}; // Must be initialized before using the driver

TEST_CASE("Test Robot Simulator", "[simulation]") {
    // Initialize resource config with the correct path
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path resourcePath = currentPath / "resources";
    if (!std::filesystem::exists(resourcePath)) {
        resourcePath = currentPath / ".." / "resources";
    }
    config::ResourceConfig::initialize(resourcePath.string());
    
    // Create required components for RobotSimulator
    auto map = std::make_shared<Map>();
    REQUIRE_NOTHROW(map->loadFromFile(config::ResourceConfig::getMapPath()));
    
    // Create a vector to store robots
    std::vector<std::shared_ptr<Robot>> robots;
    
    // Create scheduler with map and robots
    auto scheduler = std::make_shared<Scheduler>(map.get(), &robots);
    auto alertSystem = std::make_shared<AlertSystem>();
    auto dbAdapter = std::make_shared<MongoDBAdapter>("mongodb://localhost:27017", "test_db");
    
    RobotSimulator simulator(map, scheduler, alertSystem, dbAdapter);

    SECTION("Robot Management") {
        // Test adding robots
        std::cout << "Testing robot management..." << std::endl;
        REQUIRE_NOTHROW(simulator.addRobot("Robot1"));
        
        // Test robot operations
        auto& robots = simulator.getRobots();
        REQUIRE(!robots.empty());
        auto robot1 = std::find_if(robots.begin(), robots.end(),
            [](const auto& robot) { return robot->getName() == "Robot1"; });
        REQUIRE(robot1 != robots.end());
        
        // Test robot control operations
        REQUIRE_NOTHROW(simulator.startRobotCleaning("Robot1"));
        REQUIRE_NOTHROW(simulator.stopRobotCleaning("Robot1"));
        REQUIRE_NOTHROW(simulator.requestReturnToCharger("Robot1"));
        
        // Test operations with non-existent robot
        REQUIRE_THROWS(simulator.startRobotCleaning("NonexistentRobot"));
        REQUIRE_THROWS(simulator.stopRobotCleaning("NonexistentRobot"));
        REQUIRE_THROWS(simulator.requestReturnToCharger("NonexistentRobot"));
    }

    SECTION("Robot Status") {
        simulator.addRobot("TestRobot");
        
        // Get robot status
        auto statuses = simulator.getRobotStatuses();
        REQUIRE(statuses.size() == 1);
        REQUIRE(statuses[0].name == "TestRobot");
        REQUIRE(statuses[0].batteryLevel > 0);

        // Test cleaning commands
        REQUIRE_NOTHROW(simulator.startRobotCleaning("TestRobot"));
        REQUIRE_NOTHROW(simulator.stopRobotCleaning("TestRobot"));
        REQUIRE_NOTHROW(simulator.requestReturnToCharger("TestRobot"));

        // Test invalid robot commands
        REQUIRE_THROWS(simulator.startRobotCleaning("NonexistentRobot"));
        REQUIRE_THROWS(simulator.stopRobotCleaning("NonexistentRobot"));
        REQUIRE_THROWS(simulator.requestReturnToCharger("NonexistentRobot"));
    }

    SECTION("Map Access") {
        // Test map access
        const auto& map = simulator.getMap();
        REQUIRE_NOTHROW(map.getRooms());
    }
}
