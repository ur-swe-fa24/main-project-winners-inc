#include <catch2/catch_test_macros.hpp>
#include "RobotSimulator/RobotSimulator.hpp"
#include "adapter/MongoDBAdapter.hpp"
#include "Robot/Robot.h"
#include <memory>
#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <iostream>

// Create a global instance of mongocxx::instance
mongocxx::instance instance{}; // Must be initialized before using the driver

TEST_CASE("Robot Simulator Test") {
    // Create MongoDB adapter
    auto dbAdapter = std::make_shared<MongoDBAdapter>("mongodb://localhost:27017", "test_db");
    
    // Create simulator with test map
    RobotSimulator simulator(dbAdapter, "test_map.json");

    SECTION("Robot Management") {
        // Test adding robots
        std::cout << "Testing robot management..." << std::endl;
        REQUIRE_NOTHROW(simulator.addRobot("Robot1"));
        std::cout << "Added Robot1" << std::endl;
        REQUIRE_NOTHROW(simulator.addRobot("Robot2"));
        std::cout << "Added Robot2" << std::endl;

        // Verify robots were added
        auto& robots = simulator.getRobots();
        std::cout << "Number of robots: " << robots.size() << std::endl;
        REQUIRE(robots.size() == 2);

        // Test getting robot by name
        auto robot1 = simulator.getRobotByName("Robot1");
        REQUIRE(robot1 != nullptr);
        std::cout << "Found Robot1: " << robot1->getName() << std::endl;
        REQUIRE(robot1->getName() == "Robot1");

        // Test deleting robot
        REQUIRE_NOTHROW(simulator.deleteRobot("Robot1"));
        std::cout << "Deleted Robot1" << std::endl;
        REQUIRE(simulator.getRobots().size() == 1);

        // Test deleting non-existent robot
        REQUIRE_THROWS(simulator.deleteRobot("NonexistentRobot"));
        std::cout << "Successfully caught attempt to delete non-existent robot" << std::endl;
    }

    SECTION("Robot Status") {
        simulator.addRobot("TestRobot");
        
        // Get robot status
        auto statuses = simulator.getRobotStatuses();
        REQUIRE(statuses.size() == 1);
        REQUIRE(statuses[0].name == "TestRobot");
        REQUIRE(statuses[0].batteryLevel > 0);

        // Test cleaning commands
        REQUIRE_NOTHROW(simulator.startCleaning("TestRobot"));
        REQUIRE_NOTHROW(simulator.stopCleaning("TestRobot"));
        REQUIRE_NOTHROW(simulator.returnToCharger("TestRobot"));

        // Test invalid robot commands
        REQUIRE_THROWS(simulator.startCleaning("NonexistentRobot"));
        REQUIRE_THROWS(simulator.stopCleaning("NonexistentRobot"));
        REQUIRE_THROWS(simulator.returnToCharger("NonexistentRobot"));
    }

    SECTION("Map Access") {
        // Test map access
        const auto& map = simulator.getMap();
        REQUIRE_NOTHROW(map.getRooms());
    }
}
