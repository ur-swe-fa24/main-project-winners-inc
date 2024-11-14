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
    Room* startingRoom = map_.getRoomById(0);
    if (!startingRoom) {
        throw std::runtime_error("Starting room not found in the map.");
    }

    // Predefine three robots
    std::vector<std::string> predefinedRobotNames = {"SimBot-1", "SimBot-2", "SimBot-3"};
    for (const auto& robotName : predefinedRobotNames) {
        auto newRobot = std::make_shared<Robot>(robotName, 100);
        newRobot->setCurrentRoom(startingRoom);
        robots_.push_back(newRobot);

        // Save each robot's status to the database
        try {
            dbAdapter_->saveRobotStatus(newRobot);
            std::cout << "Predefined robot '" << robotName << "' added and saved to database." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Exception while saving robot '" << robotName << "' status: " << e.what() << std::endl;
        }
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

// Define getMap() methods
Map& RobotSimulator::getMap() {
    return map_;
}

const Map& RobotSimulator::getMap() const {
    return map_;
}

// Define getRobots() methods
std::vector<std::shared_ptr<Robot>>& RobotSimulator::getRobots() {
    return robots_;
}

const std::vector<std::shared_ptr<Robot>>& RobotSimulator::getRobots() const {
    return robots_;
}

void RobotSimulator::simulationLoop() {
    while (running_) {
        std::vector<std::shared_ptr<Robot>> robotsCopy;

        // Copy the robots vector while holding the mutex
        {
            std::lock_guard<std::mutex> lock(robotsMutex_);
            robotsCopy = robots_;
        }

        // Iterate over the copied robots
        for (auto& robot : robotsCopy) {
            bool needsSave = false;
            bool generateLowBatteryAlert = false;
            bool generateChargingAlert = false;

            // Update robot state within a mutex lock
            {
                std::lock_guard<std::mutex> lock(robotsMutex_);

                // Update the robot
                robot->update(map_);

                if (robot->isCleaning()) {
                    needsSave = true;
                }

                if (robot->getBatteryLevel() < 20 && !robot->isLowBatteryAlertSent()) {
                    generateLowBatteryAlert = true;
                    robot->setLowBatteryAlertSent(true);

                    // Send robot to charging station
                    if (!robot->isCharging()) {
                        Room* chargingStation = map_.getRoomById(0);
                        if (chargingStation) {
                            std::vector<int> pathToCharger = map_.getRoute(*robot->getCurrentRoom(), *chargingStation);
                            if (!pathToCharger.empty()) {
                                robot->setMovementPath(pathToCharger, map_);
                                robot->setTargetRoom(chargingStation);
                            } else {
                                std::cerr << "Robot " << robot->getName() << ": Cannot find path to charging station." << std::endl;
                            }
                        }
                    }
                }

                if (robot->getBatteryLevel() <= 20 && !robot->isCharging()) {
                    // Send robot to charging station
                    Room* chargingStation = map_.getRoomById(0);
                    if (chargingStation) {
                        std::vector<int> pathToCharger = map_.getRoute(*robot->getCurrentRoom(), *chargingStation);
                        if (!pathToCharger.empty()) {
                            robot->setMovementPath(pathToCharger, map_);
                            robot->setTargetRoom(chargingStation);
                        } else {
                            std::cerr << "Robot " << robot->getName() << ": Cannot find path to charging station." << std::endl;
                        }
                    }
                }

                // Start charging when at charging station
                if (robot->getCurrentRoom() && robot->getCurrentRoom()->getRoomId() == 0 && !robot->isCharging() && robot->getBatteryLevel() < 10) {
                    robot->startCharging();
                    std::cout << "Robot " << robot->getName() << " started charging at the charging station." << std::endl;
                }
            }  // Mutex lock is released here

            // Perform database operations without holding the mutex
            try {
                if (needsSave) {
                    dbAdapter_->saveRobotStatus(robot);
                }

                if (generateLowBatteryAlert) {
                    auto room = std::make_shared<Room>("Simulation Area", 0);
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
                std::cerr << "RobotSimulator: Exception during database operation: " << e.what() << std::endl;
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
        status.status = robot->getStatus();
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
            if (!robot->isCharging()) {
                robot->recharge(); // Stops current tasks and sends robot to charger
                std::cout << "Robot " << robotName << " is returning to charger." << std::endl;
            } else {
                std::cout << "Robot " << robotName << " is already charging." << std::endl;
            }
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

void RobotSimulator::addRobot(const std::string& robotName) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    // Check if a robot with the same name already exists
    for (const auto& robot : robots_) {
        if (robot->getName() == robotName) {
            throw std::runtime_error("Robot with name '" + robotName + "' already exists.");
        }
    }

    // Initialize the new robot at the starting room
    Room* startingRoom = map_.getRoomById(0);  // Assuming room with ID 0 is the charging station
    if (!startingRoom) {
        throw std::runtime_error("Starting room not found in the map.");
    }

    auto newRobot = std::make_shared<Robot>(robotName, 100);
    newRobot->setCurrentRoom(startingRoom);
    robots_.push_back(newRobot);

    // Save the new robot's status to the database
    try {
        dbAdapter_->saveRobotStatus(newRobot);
    } catch (const std::exception& e) {
        std::cerr << "Exception while saving new robot status: " << e.what() << std::endl;
    }
}

void RobotSimulator::deleteRobot(const std::string& robotName) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    auto it = std::find_if(robots_.begin(), robots_.end(),
                           [&robotName](const std::shared_ptr<Robot>& robot) {
                               return robot->getName() == robotName;
                           });
    if (it != robots_.end()) {
        // Remove the robot from the vector
        robots_.erase(it);

        // Optionally, remove the robot's data from the database
        // You would need to implement dbAdapter_->deleteRobotStatus(robotName);

        // For now, we'll just log the deletion
        std::cout << "Robot '" << robotName << "' deleted from the simulator." << std::endl;
    } else {
        throw std::runtime_error("Robot with name '" + robotName + "' does not exist.");
    }
}
