#include "RobotSimulator/RobotSimulator.hpp"
#include <chrono>
#include <iostream>

RobotSimulator::RobotSimulator(std::shared_ptr<MongoDBAdapter> dbAdapter)
    : dbAdapter_(dbAdapter), running_(false) {
    // Initialize robots; you can retrieve from db or create new ones
    robots_.push_back(std::make_shared<Robot>("SimBot-1", 100));
    robots_.push_back(std::make_shared<Robot>("SimBot-2", 100));
    robots_.push_back(std::make_shared<Robot>("SimBot-3", 100));

    // Save initial robots to the database
    try {
        for (const auto& robot : robots_) {
            dbAdapter_->saveRobotStatus(robot);
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in RobotSimulator constructor while saving robot status: " << e.what() << std::endl;
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
        // Iterate over the robots
        for (auto& robot : robots_) {
            bool needsSave = false;
            bool generateLowBatteryAlert = false;
            bool generateChargingAlert = false;

            // Update robot state within a mutex lock
            {
                std::lock_guard<std::mutex> lock(robotsMutex_);
                if (robot->isCleaning()) {
                    robot->depleteBattery(1);  // Deplete battery by 1%
                    needsSave = true;
                }

                // Check battery level for low battery alert
                if (robot->getBatteryLevel() < 20 && !robot->isLowBatteryAlertSent()) {
                    generateLowBatteryAlert = true;
                    robot->setLowBatteryAlertSent(true);
                }

                // If battery is zero or below, recharge the robot
                if (robot->getBatteryLevel() <= 0) {
                    robot->recharge();
                    needsSave = true;
                    generateChargingAlert = true;
                }
            }  // Mutex lock is released here

            // Perform database operations without holding the mutex
            try {
                if (needsSave) {
                    dbAdapter_->saveRobotStatus(robot);
                }

                if (generateLowBatteryAlert) {
                    auto room = std::make_shared<Room>("Simulation Area", 1);
                    Alert alert(
                        "Low Battery",
                        "Robot " + robot->getName() + " battery level critical: " +
                            std::to_string(robot->getBatteryLevel()) + "%",
                        robot, room, std::time(nullptr));

                    dbAdapter_->saveAlert(alert);
                }

                if (generateChargingAlert) {
                    auto room = std::make_shared<Room>("Charging Station", 0);
                    Alert alert(
                        "Charging",
                        "Robot " + robot->getName() + " has returned to charger",
                        robot, room, std::time(nullptr));

                    dbAdapter_->saveAlert(alert);
                }
            } catch (const std::exception& e) {
                std::cerr << "Exception during database operation: " << e.what() << std::endl;
            }
        }

        // Sleep before the next simulation step
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

std::vector<RobotSimulator::RobotStatus> RobotSimulator::getRobotStatuses() {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    std::vector<RobotStatus> statuses;
    for (const auto& robot : robots_) {
        RobotStatus status;
        status.name = robot->getName();
        status.batteryLevel = robot->getBatteryLevel();
        status.isCleaning = robot->isCleaning();
        statuses.push_back(status);
    }
    return statuses;
}

std::shared_ptr<Robot> RobotSimulator::getRobotByName(const std::string& name) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    for (const auto& robot : robots_) {
        if (robot->getName() == name) {
            return robot;
        }
    }
    return nullptr;  // Return nullptr if not found
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
