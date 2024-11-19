#define CATCH_CONFIG_MAIN
#include "RobotSimulator/RobotSimulator.hpp"
#include "Room/Room.h"
#include "Robot/Robot.h"
#include "alert/Alert.h"
#include "adapter/MongoDBAdapter.hpp" 
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <chrono>
#include <ctime>
#include <memory>
#include <thread>
#include <vector>
#include <mongocxx/instance.hpp>
#include "virtual_wall/virtual_wall.h"


// Create a global instance of mongocxx::instance
mongocxx::instance instance{}; // Must be initialized before using the driver

TEST_CASE("Robot Simulation Test") {
    // Initialize MongoDB Adapter if needed
    std::string uri = "mongodb://localhost:27017";
    std::string dbName = "simulationDB";
    MongoDBAdapter dbAdapter(uri, dbName);

    // Initialize Room instances and neighbors
    std::vector<Room *> neighbors;
    Room room1("Living Room", 1, "Carpet", true);
    Room room2("Kitchen", 2, "Tile", false);
    Room room3("Bedroom", 2, "Tile",false);

    // Connect rooms (if applicable)
    room1.addNeighbor(&room2);
    room2.addNeighbor(&room3);

    // Initialize Robot and Simulation
    auto robot = std::make_shared<Robot>("CleaningRobot", room1);
    auto simulation = std::make_shared<RobotSimulator>();

    SECTION("Robot can move between connected rooms") {
        // Ensure the robot starts in room1
        REQUIRE(robot->getCurrentRoom() == &room1);

        // Move robot to room2
        robot->moveToRoom(&room2);
        REQUIRE(robot->getCurrentRoom() == &room2);

        // Move robot to room3
        robot->moveToRoom(&room3);
        REQUIRE(robot->getCurrentRoom() == &room3);
    }

    SECTION("Robot cannot move through virtual walls") {
        // Simulate a virtual wall between room1 and room2
        VirtualWall(&room2, &room3);

        // Attempt to move the robot from room1 to room2 (blocked by virtual wall)
        bool moveResult = robot->moveToRoom(&room2);;
        REQUIRE_FALSE(moveResult);  // Move should fail due to virtual wall
    }

    SECTION("Delete all simulation data from database") {
        dbAdapter.deleteAllAlerts();
        dbAdapter.deleteAllRobotStatuses();

        auto alertsAfterDeletion = dbAdapter.retrieveAlerts();
        auto robotsAfterDeletion = dbAdapter.retrieveRobotStatuses();

        REQUIRE(alertsAfterDeletion.empty());
        REQUIRE(robotsAfterDeletion.empty());

        dbAdapter.dropAlertCollection();
        dbAdapter.dropRobotStatusCollection();
    }

    simulation->stop();  // Stop simulation if necessary
}
