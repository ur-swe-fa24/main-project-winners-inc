#include "RobotSimulator/RobotSimulator.hpp"
#include <chrono>
#include <iostream>
#include <queue>
#include <set>

RobotSimulator::RobotSimulator(std::shared_ptr<MongoDBAdapter> dbAdapter, const std::string& mapFile)
    : dbAdapter_(dbAdapter), running_(false) {
    // Load the map
    map_.loadFromFile(mapFile);

    // Initialize rooms in the database
    dbAdapter_->initializeRooms(map_.getRooms());

    // Initialize starting room
    Room* startingRoom = map_.getRoomById(0); // Assuming room ID 0 is the charging station
    if (!startingRoom) {
        throw std::runtime_error("Starting room not found in the map.");
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

Map& RobotSimulator::getMap() {
    return map_;
}

const Map& RobotSimulator::getMap() const {
    return map_;
}

std::vector<std::shared_ptr<Robot>>& RobotSimulator::getRobots() {
    return robots_;
}

const std::vector<std::shared_ptr<Robot>>& RobotSimulator::getRobots() const {
    return robots_;
}

std::vector<RobotSimulator::RobotStatus> RobotSimulator::getRobotStatuses() {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    std::vector<RobotStatus> statuses;
    for (const auto& robot : robots_) {
        std::string currentRoomName = robot->getCurrentRoom() ? robot->getCurrentRoom()->getRoomName() : "Unknown";
        statuses.emplace_back(robot->getName(),
                              robot->getBatteryLevel(),
                              robot->getWaterLevel(),
                              robot->getStatus(),
                              currentRoomName);
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
    bool found = false;
    for (auto& robot : robots_) {
        if (robot->getName() == robotName) {
            robot->startCleaning();
            found = true;
            break;
        }
    }
    if (!found) {
        throw std::runtime_error("Robot not found: " + robotName);
    }
    cv_.notify_one();
}

void RobotSimulator::stopCleaning(const std::string& robotName) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    bool found = false;
    for (auto& robot : robots_) {
        if (robot->getName() == robotName) {
            robot->stopCleaning();
            found = true;
            break;
        }
    }
    if (!found) {
        throw std::runtime_error("Robot not found: " + robotName);
    }
    cv_.notify_one();
}

void RobotSimulator::returnToCharger(const std::string& robotName) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    bool found = false;
    for (auto& robot : robots_) {
        if (robot->getName() == robotName) {
            if (!robot->isCharging()) {
                robot->recharge(map_); // Pass the map object
                std::cout << "Robot " << robotName << " is returning to charger." << std::endl;
            } else {
                std::cout << "Robot " << robotName << " is already charging." << std::endl;
            }
            found = true;
            break;
        }
    }
    if (!found) {
        throw std::runtime_error("Robot not found: " + robotName);
    }
    cv_.notify_one();
}

void RobotSimulator::simulationLoop() {
    const double UPDATE_INTERVAL = 0.1; // 100ms update interval
    
    while (running_) {
        std::vector<std::shared_ptr<Robot>> robotsCopy;

        // Copy the robots vector while holding the mutex
        {
            std::lock_guard<std::mutex> lock(robotsMutex_);
            robotsCopy = robots_;
        }

        // Iterate over the copied robots
        for (auto& robot : robotsCopy) {
            std::lock_guard<std::mutex> lock(robotsMutex_);
            
            // Update robot state
            robot->update(map_);

            // Check if robot needs charging
            if (robot->needsCharging() && !robot->isCharging()) {
                returnToCharger(robot->getName());
            }

            // Check if robot needs water refill
            if (robot->needsWaterRefill() && robot->isCleaning()) {
                robot->stopCleaning();
                // TODO: Implement water refill station logic
            }

            // Check if the robot has cleaned a room
            Room* currentRoom = robot->getCurrentRoom();
            if (currentRoom && robot->isCleaning()) {
                currentRoom->isRoomClean = true;
                // Save the room status asynchronously
                dbAdapter_->saveRoomStatusAsync(*currentRoom);
            }

            // Handle alerts
            if (robot->needsCharging() && !robot->isLowBatteryAlertSent()) {
                std::cout << "Low battery alert for " << robot->getName() << std::endl;
                robot->setLowBatteryAlertSent(true);
            }

            if (robot->needsWaterRefill() && !robot->isLowWaterAlertSent()) {
                std::cout << "Low water alert for " << robot->getName() << std::endl;
                robot->setLowWaterAlertSent(true);
            }

            // Reset alert flags when resources are replenished
            if (!robot->needsCharging()) {
                robot->setLowBatteryAlertSent(false);
            }

            if (!robot->needsWaterRefill()) {
                robot->setLowWaterAlertSent(false);
            }
        }

        // Sleep for update interval
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(UPDATE_INTERVAL * 1000)));
    }
}

void RobotSimulator::addRobot(const std::string& robotName) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    // Check if robot with this name already exists
    auto it = std::find_if(robots_.begin(), robots_.end(),
        [&robotName](const std::shared_ptr<Robot>& robot) {
            return robot->getName() == robotName;
        });
    
    if (it != robots_.end()) {
        throw std::runtime_error("Robot with name '" + robotName + "' already exists");
    }
    
    // Create new robot at starting room (charging station)
    Room* startingRoom = map_.getRoomById(0);
    if (!startingRoom) {
        throw std::runtime_error("Starting room (charging station) not found in the map");
    }
    
    // Initialize robot with full battery and water
    auto newRobot = std::make_shared<Robot>(robotName, 100.0, 100.0);
    newRobot->setCurrentRoom(startingRoom);
    robots_.push_back(newRobot);
    
    // Log robot creation
    std::cout << "Added new robot: " << robotName << " at charging station." << std::endl;
}

void RobotSimulator::deleteRobot(const std::string& robotName) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    // Find robot with the given name
    auto it = std::find_if(robots_.begin(), robots_.end(),
        [&robotName](const std::shared_ptr<Robot>& robot) {
            return robot->getName() == robotName;
        });
    
    if (it == robots_.end()) {
        throw std::runtime_error("Robot with name '" + robotName + "' not found");
    }
    
    // Remove robot from vector
    robots_.erase(it);
    
    // Log robot deletion
    std::cout << "Deleted robot: " << robotName << std::endl;
}
