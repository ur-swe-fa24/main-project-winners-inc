#ifndef ROBOT_SIMULATOR_HPP
#define ROBOT_SIMULATOR_HPP

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "Robot/Robot.h"
#include "adapter/MongoDBAdapter.hpp"

class RobotSimulator {
public:
    RobotSimulator(std::shared_ptr<MongoDBAdapter> dbAdapter);
    ~RobotSimulator();

    void start();
    void stop();

    // Methods to interact with the simulator
    std::vector<std::shared_ptr<Robot>> getRobots();
    std::shared_ptr<Robot> getRobotByName(const std::string& name);
    void startCleaning(const std::string& robotName);
    void stopCleaning(const std::string& robotName);
    void returnToCharger(const std::string& robotName);


private:
    void simulationLoop();

    std::shared_ptr<MongoDBAdapter> dbAdapter_;
    std::vector<std::shared_ptr<Robot>> robots_;

    std::thread simulationThread_;
    std::atomic<bool> running_;

    // Concurrency control
    std::mutex robotsMutex_;
    std::condition_variable cv_;
};

#endif // ROBOT_SIMULATOR_HPP
