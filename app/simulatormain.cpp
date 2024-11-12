#include "RobotSimulator/RobotSimulator.hpp"
#include "adapter/MongoDBAdapter.hpp"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include <mongocxx/instance.hpp>
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<bool> keepRunning(true);

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    keepRunning = false;
}

int main() {
    // Register signal handler for graceful shutdown
    std::signal(SIGINT, signalHandler);

    try {
        // Initialize MongoDB instance
        mongocxx::instance mongoInstance{};

        // Initialize MongoDBAdapter
        std::string uri = "mongodb://localhost:27017";
        std::string dbName = "mydb_simulator_test";
        auto dbAdapter = std::make_shared<MongoDBAdapter>(uri, dbName);

        // Create and start the simulator
        auto simulator = std::make_shared<RobotSimulator>(dbAdapter);
        simulator->start();

        std::cout << "Robot Simulator running. Press Ctrl+C to exit.\n";

        // Main loop
        while (keepRunning) {
            // Get the current state of robots
            auto robots = simulator->getRobotStatuses();

            // Display robot statuses
            std::cout << "\nCurrent Robot Statuses:\n";
            for (const auto& robot : robots) {
                std::cout << "Robot Name: " << robot.name
                          << ", Battery Level: " << robot.batteryLevel << "%"
                          << ", Cleaning: " << (robot.isCleaning ? "Yes" : "No") << "\n";
            }

            // Sleep for a while
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        // Stop the simulator
        simulator->stop();

        std::cout << "Simulator stopped. Exiting.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error in simulator: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
