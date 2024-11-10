#include "RobotSimulator/RobotSimulator.hpp"
#include <chrono>
#include <iostream>

RobotSimulator::RobotSimulator(std::shared_ptr<MongoDBAdapter> dbAdapter)
    : dbAdapter_(dbAdapter), running_(false) {
    // Initialize robots, you can retrieve from db or create new ones
    robots_.push_back(std::make_shared<Robot>("SimBot-1", 100));
    robots_.push_back(std::make_shared<Robot>("SimBot-2", 100));
    robots_.push_back(std::make_shared<Robot>("SimBot-3", 100));

    // Save initial robots to database
    for (const auto& robot : robots_) {
        dbAdapter_->saveRobotStatusAsync(robot);
    }
}

RobotSimulator::~RobotSimulator() {
    stop();
}

void RobotSimulator::start() {
    running_ = true;
    simulationThread_ = std::thread(&RobotSimulator::simulationLoop, this);
}

void RobotSimulator::stop() {
    running_ = false;
    cv_.notify_all();
    if (simulationThread_.joinable()) {
        simulationThread_.join();
    }
}

void RobotSimulator::simulationLoop() {
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(robotsMutex_);
            // Simulate robots' battery decreasing
            for (auto& robot : robots_) {
                if (robot->isCleaning()) {
                    robot->depleteBattery(1); // Deplete battery by 1%
                }

                dbAdapter_->saveRobotStatusAsync(robot);

                // If battery level is below threshold, generate alert
                if (robot->getBatteryLevel() < 20 && !robot->isLowBatteryAlertSent()) {
                    auto room = std::make_shared<Room>("Simulation Area", 1);
                    Alert alert("Low Battery",
                                "Robot " + robot->getName() + " battery level critical: " + std::to_string(robot->getBatteryLevel()) + "%",
                                robot, room, std::time(nullptr));
                    dbAdapter_->saveAlert(alert);
                    robot->setLowBatteryAlertSent(true);
                }

                // If battery is zero, recharge the robot
                if (robot->getBatteryLevel() <= 0) {
                    robot->recharge();
                    dbAdapter_->saveRobotStatusAsync(robot);

                    auto room = std::make_shared<Room>("Charging Station", 0);
                    Alert alert("Charging", "Robot " + robot->getName() + " has returned to charger",
                                robot, room, std::time(nullptr));
                    dbAdapter_->saveAlert(alert);
                }
            }
        }

        // Sleep for a while before next simulation step
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

std::vector<std::shared_ptr<Robot>> RobotSimulator::getRobots() {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    return robots_;
}

void RobotSimulator::startCleaning(const std::string& robotName) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    for (auto& robot : robots_) {
        if (robot->getName() == robotName) {
            robot->startCleaning();
            break;
        }
    }
    cv_.notify_one();
}

void RobotSimulator::stopCleaning(const std::string& robotName) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    for (auto& robot : robots_) {
        if (robot->getName() == robotName) {
            robot->stopCleaning();
            break;
        }
    }
    cv_.notify_one();
}

void RobotSimulator::returnToCharger(const std::string& robotName) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    for (auto& robot : robots_) {
        if (robot->getName() == robotName) {
            robot->recharge();
            break;
        }
    }
    cv_.notify_one();
}
