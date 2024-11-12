// simulatormain.cpp

#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <thread>
#include <memory>
#include "RobotSimulator/RobotSimulator.hpp"
#include "adapter/MongoDBAdapter.hpp"

int main() {
    try {
        // Initialize MongoDB instance
        mongocxx::instance instance{};

        // Initialize MongoDB Adapter
        auto dbAdapter = std::make_shared<MongoDBAdapter>("mongodb://localhost:27017", "mydb9");

        // Specify the map file path
        std::string mapFile = "map.json"; // Adjust the path as necessary
        // Create the simulator


        auto simulator = std::make_shared<RobotSimulator>(dbAdapter, mapFile);

        // Start the simulator
        simulator->start();
        // Run the simulator for a certain amount of time
        std::this_thread::sleep_for(std::chrono::seconds(30)); // Run for 30 seconds

        // Stop the simulator
        simulator->stop();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
        return -1;
    }
}
