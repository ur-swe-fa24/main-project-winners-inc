#include "RobotSimulator/RobotSimulator.hpp"
#include "adapter/MongoDBAdapter.hpp"
#include <mongocxx/instance.hpp>
#include <csignal>
#include <iostream>
#include <chrono>
#include <iomanip>

std::atomic<bool> keepRunning(true);

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    keepRunning = false;
}

// Helper function to print current time
std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");
    return ss.str();
}

int main() {
    try {
        // Register signal handler
        std::signal(SIGINT, signalHandler);

        std::cout << "[" << getCurrentTime() << "] Initializing Robot Simulator..." << std::endl;

        // Initialize MongoDB instance
        mongocxx::instance mongoInstance{};

        // Initialize MongoDBAdapter
        std::string uri = "mongodb://localhost:27017";
        std::string dbName = "mydb8";
        auto dbAdapter = std::make_shared<MongoDBAdapter>(uri, dbName);
        std::cout << "[" << getCurrentTime() << "] Connected to MongoDB: " << dbName << std::endl;

        // Create and start the simulator
        RobotSimulator simulator(dbAdapter, "map.json");
        std::cout << "[" << getCurrentTime() << "] Created simulator with map.json" << std::endl;

        // Add some test robots
        simulator.addRobot("Robot1");
        simulator.addRobot("Robot2");
        std::cout << "[" << getCurrentTime() << "] Added test robots: Robot1, Robot2" << std::endl;

        simulator.start();
        std::cout << "[" << getCurrentTime() << "] Simulator started" << std::endl;

        // Keep the simulator running until user interrupts (Ctrl+C)
        std::cout << "[" << getCurrentTime() << "] Robot Simulator running. Press Ctrl+C to exit." << std::endl;
        
        int tick = 0;
        while (keepRunning) {
            if (tick % 5 == 0) { // Print status every 5 seconds
                auto robots = simulator.getRobots();
                std::cout << "\n[" << getCurrentTime() << "] Current Robot Status:" << std::endl;
                for (const auto& robot : robots) {
                    std::cout << "  - " << robot->getName() 
                            << " | Battery: " << robot->getBatteryLevel() << "%"
                            << " | Status: " << robot->getStatus()
                            << " | Room: " << (robot->getCurrentRoom() ? robot->getCurrentRoom()->getRoomName() : "None")
                            << std::endl;
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            tick++;
        }

        // Stop the simulator
        std::cout << "\n[" << getCurrentTime() << "] Stopping simulator..." << std::endl;
        simulator.stop();
        std::cout << "[" << getCurrentTime() << "] Simulator stopped" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[" << getCurrentTime() << "] Error: " << e.what() << std::endl;
        return 1;
    }
}
