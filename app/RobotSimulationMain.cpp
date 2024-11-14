#include "RobotSimulator.hpp"
#include <mongocxx/instance.hpp>
#include <csignal>
#include <iostream>

std::atomic<bool> keepRunning(true);

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    keepRunning = false;
}

int main() {
    // Register signal handler
    std::signal(SIGINT, signalHandler);

    // Initialize MongoDB instance
    mongocxx::instance mongoInstance{};

    // Initialize MongoDBAdapter
    std::string uri = "mongodb://localhost:27017";
    std::string dbName = "mydb8";
    auto dbAdapter = std::make_shared<MongoDBAdapter>(uri, dbName);

    // Create and start the simulator
    RobotSimulator simulator(dbAdapter);
    simulator.start();

    // Keep the simulator running until user interrupts (Ctrl+C)
    std::cout << "Robot Simulator running. Press Ctrl+C to exit." << std::endl;
    while (keepRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Stop the simulator
    simulator.stop();

    return 0;
}
