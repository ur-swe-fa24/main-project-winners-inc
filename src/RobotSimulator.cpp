// RobotSimulator.cpp

#include "RobotSimulator/RobotSimulator.hpp"
#include <chrono>
#include <iostream>
#include <queue>
#include <set>

RobotSimulator::RobotSimulator(std::shared_ptr<MongoDBAdapter> dbAdapter, const std::string& mapFile)
    : dbAdapter_(dbAdapter), running_(false) {
    // Load the map
    map_.loadFromFile(mapFile);

    // Initialize robots at starting room
    Room* startingRoom = map_.getRoomById(1);
    if (!startingRoom) {
        throw std::runtime_error("Starting room not found in the map.");
    }

    robots_.push_back(std::make_shared<Robot>("SimBot-1", 100));
    robots_.back()->setCurrentRoom(startingRoom);

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
        simulateRobotMovement();

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
        status.currentRoomName = robot->getCurrentRoom() ? robot->getCurrentRoom()->getRoomName() : "Unknown";
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

void RobotSimulator::simulateRobotMovement() {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    for (auto& robot : robots_) {
        if (robot->isCleaning()) {
            Room* currentRoom = robot->getCurrentRoom();
            Room* nextRoom = getNextRoomToClean(currentRoom);
            if (nextRoom && robot->moveToRoom(nextRoom)) {
                // Mark the room as clean
                nextRoom->markClean();
                // Optionally, save robot status to the database
                dbAdapter_->saveRobotStatus(robot);
            }
        }
    }
}

Room* RobotSimulator::getNextRoomToClean(Room* currentRoom) {
    // Implement logic to decide the next room
    // For example, find the nearest dirty room
    std::queue<Room*> queue;
    std::set<Room*> visited;
    queue.push(currentRoom);
    visited.insert(currentRoom);

    while (!queue.empty()) {
        Room* room = queue.front();
        queue.pop();

        if (!room->isRoomClean) {
            return room;
        }

        for (Room* neighbor : room->neighbors) {
            if (visited.find(neighbor) == visited.end()) {
                queue.push(neighbor);
                visited.insert(neighbor);
            }
        }
    }
    return nullptr;  // No dirty rooms found
}

const std::vector<std::shared_ptr<Robot>>& RobotSimulator::getRobots() const {
    return robots_;
}

const Map& RobotSimulator::getMap() const {
    return map_;
}
