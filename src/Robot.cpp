#include "Robot/Robot.h"
#include "map/map.h"
#include <iostream>
#include <algorithm>

// Constructor implementation
Robot::Robot(const std::string& name, double batteryLevel, double waterLevel)
    : name(name), 
      batteryLevel(batteryLevel),
      waterLevel_(waterLevel),
      cleaning_(false),
      lowBatteryAlertSent_(false),
      lowWaterAlertSent_(false),
      currentRoom_(nullptr),
      nextRoom_(nullptr),
      targetRoom_(nullptr),
      isCharging_(false),
      chargingTimeRemaining_(0.0),
      movementProgress_(0.0),
      cleaningTimeRemaining_(0.0),
      returningToCharger_(false),
      hasPendingTasks_(false) {}

// Methods to start and stop cleaning
void Robot::startCleaning() {
    if (waterLevel_ < 10.0) {
        std::cout << "Warning: Water level too low to start cleaning!" << std::endl;
        return;
    }
    
    cleaning_ = true;
    if (currentRoom_) {
        // Adjust cleaning time based on room type
        if (currentRoom_->flooringType == "Hardwood") {
            cleaningTimeRemaining_ = 15.0;
            depleteWater(5.0); // Hardwood floors use less water
        } else if (currentRoom_->flooringType == "Carpet") {
            cleaningTimeRemaining_ = 20.0;
            depleteWater(10.0); // Carpet cleaning uses more water
        } else {
            cleaningTimeRemaining_ = 10.0;
            depleteWater(7.0); // Default water usage
        }
    } else {
        std::cerr << "Error: currentRoom_ is null in startCleaning()" << std::endl;
        cleaningTimeRemaining_ = 10.0;
        depleteWater(7.0);
    }
}

void Robot::stopCleaning() {
    cleaning_ = false;
}

bool Robot::isCleaning() const {
    return cleaning_;
}

// Low battery alert methods
void Robot::setLowBatteryAlertSent(bool sent) {
    lowBatteryAlertSent_ = sent;
}

bool Robot::isLowBatteryAlertSent() const {
    return lowBatteryAlertSent_;
}

// Water management methods
void Robot::refillWater() {
    waterLevel_ = 100.0;
    lowWaterAlertSent_ = false;
    std::cout << "Robot " << name << " water tank refilled to 100%" << std::endl;
}

void Robot::depleteWater(double amount) {
    waterLevel_ = std::max(0.0, waterLevel_ - amount);
    if (waterLevel_ < 20.0 && !lowWaterAlertSent_) {
        std::cout << "Warning: " << name << " water level low (" << waterLevel_ << "%)" << std::endl;
        lowWaterAlertSent_ = true;
    }
}

double Robot::getWaterLevel() const {
    return waterLevel_;
}

bool Robot::needsWaterRefill() const {
    return waterLevel_ < 20.0;
}

void Robot::setLowWaterAlertSent(bool sent) {
    lowWaterAlertSent_ = sent;
}

bool Robot::isLowWaterAlertSent() const {
    return lowWaterAlertSent_;
}

// Send status update
void Robot::sendStatusUpdate() const {
    std::cout << "Robot " << name << " Status:" << std::endl
              << "Battery Level: " << batteryLevel << "%" << std::endl
              << "Water Level: " << waterLevel_ << "%" << std::endl;
}

// Recharge battery
// Initiate recharge by sending robot to charger
void Robot::recharge(const Map& map) {
    if (isCharging_) {
        std::cout << "Robot " << name << " is already charging." << std::endl;
        return;
    }

    // Find the charging station (room ID 0)
    Room* charger = map.getRoomById(0);
    if (!charger) {
        std::cerr << "Robot " << name << ": Charging station not found." << std::endl;
        return;
    }

    if (currentRoom_ == charger) {
        // Already at charger, start charging
        startCharging();
        std::cout << "Robot " << name << " is already at the charger and started charging." << std::endl;
    } else {
        // Get path to charger
        std::vector<int> pathToCharger = map.getRoute(*currentRoom_, *charger);
        if (!pathToCharger.empty()) {
            setMovementPath(pathToCharger, map);
            setTargetRoom(charger);
            std::cout << "Robot " << name << " is moving to the charger." << std::endl;
        } else {
            std::cerr << "Robot " << name << ": No path to charger found." << std::endl;
        }
    }
}

// Check if robot needs maintenance
bool Robot::needsMaintenance() const {
    if (batteryLevel < 20.0) {
        std::cout << "Robot " << name << " needs maintenance (battery level is low)." << std::endl;
        return true;
    }
    return false;
}

// Getters
std::string Robot::getName() const {
    return name;
}

double Robot::getBatteryLevel() const {
    return batteryLevel;
}

// Deplete battery
void Robot::depleteBattery(double amount) {
    batteryLevel = std::max(0.0, batteryLevel - amount);
    if (batteryLevel < 20.0 && !lowBatteryAlertSent_) {
        std::cout << "Warning: " << name << " battery level low (" << batteryLevel << "%)" << std::endl;
        lowBatteryAlertSent_ = true;
    }
}

// Current room management
void Robot::setCurrentRoom(Room* room) {
    currentRoom_ = room;
}

Room* Robot::getCurrentRoom() const {
    return currentRoom_;
}

double Robot::getMovementProgress() const {
    return movementProgress_;
}

Room* Robot::getNextRoom() const {
    return nextRoom_;
}

void Robot::setTargetRoom(Room* room) {
    targetRoom_ = room;
}

// Move to adjacent room
bool Robot::moveToRoom(Room* room) {
    if (std::find(currentRoom_->neighbors.begin(), currentRoom_->neighbors.end(), room) != currentRoom_->neighbors.end()) {
        currentRoom_ = room;
        return true;
    }
    // Check for virtual walls (if implemented)
    // ...
    return false;  // Can't move to the room
}

// Set movement path
void Robot::setMovementPath(const std::vector<int>& roomIds, const Map& map) {
    if (movementProgress_ > 0.0 || !movementQueue_.empty() || cleaning_) {
    // Robot is busy; do not interrupt current path
    std::cout << "Robot " << name << " is busy and cannot accept a new movement path right now." << std::endl;
    return;
    }

    // Clear the existing movement queue
    while (!movementQueue_.empty()) {
        movementQueue_.pop();
    }

    std::cout << "Robot " << name << ": Setting movement path: ";

    // Iterate through the room IDs to set the movement path
    for (size_t i = 0; i < roomIds.size(); ++i) {
        int roomId = roomIds[i];
        Room* room = map.getRoomById(roomId);
        if (room) {
            // Skip the first room if it is the current room
            if (i == 0 && room == currentRoom_) {
                std::cout << "(current) ";
                continue;
            }
            movementQueue_.push(room);
            std::cout << roomId << " ";
        } else {
            std::cerr << "Robot " << name << ": Room ID " << roomId << " does not exist in the map.\n";
        }
    }

    std::cout << std::endl;

    // If the robot is currently charging, stop charging
    if (isCharging_) {
        stopCharging();
        std::cout << "Robot " << name << " was charging. Stopped charging to start moving." << std::endl;
    }
}

void Robot::update(const Map& map) {
    if (isCharging_) {
        // Recharge battery and refill water when at charging station
        batteryLevel = std::min(100.0, batteryLevel + 10.0);
        waterLevel_ = std::min(100.0, waterLevel_ + 10.0);

        std::cout << "Robot " << name << " is charging. Battery level: " << batteryLevel
                  << "%, Water level: " << waterLevel_ << "%" << std::endl;

        if (batteryLevel >= 100.0 && waterLevel_ >= 100.0) {
            stopCharging();
            returningToCharger_ = false;
            std::cout << "Robot " << name << " finished charging and refilling water." << std::endl;
            if (hasPendingTasks_) {
                // Resume tasks
                if (savedTask_) {
                    taskQueue_.push(savedTask_);
                    savedTask_.reset();
                    hasPendingTasks_ = true; // Set to true to indicate pending tasks
                } else {
                    hasPendingTasks_ = false;
                }
                std::cout << "Robot " << name << " is resuming tasks." << std::endl;
            }
        }
        return;
    }

    // Check battery and water levels before any action
    if (batteryLevel <= 20.0 || waterLevel_ <= 0.0) {
        // Battery or water low, return to charger
        if (!returningToCharger_) {
            returningToCharger_ = true;
            hasPendingTasks_ = !taskQueue_.empty() || cleaning_ || movementProgress_ > 0.0;

            // Save current task if cleaning
            if (cleaning_) {
                if (currentCleaningTask_) {
                    savedTask_ = currentCleaningTask_;
                }
                cleaning_ = false;
                cleaningTimeRemaining_ = 0.0;
            }

            // Clear movement queue and current movement
            while (!movementQueue_.empty()) {
                movementQueue_.pop();
            }
            movementProgress_ = 0.0;
            nextRoom_ = nullptr;

            // Navigate to charger
            Room* chargingStation = map.getRoomById(0);
            if (chargingStation) {
                std::vector<int> pathToCharger = map.getRoute(*currentRoom_, *chargingStation);
                if (!pathToCharger.empty()) {
                    setMovementPath(pathToCharger, map);
                    setTargetRoom(chargingStation);
                    std::cout << "Robot " << name << " is returning to charging station due to low "
                              << (batteryLevel <= 30.0 ? "battery (" + std::to_string(batteryLevel) + "%)" 
                                                     : "water (" + std::to_string(waterLevel_) + "%)") << std::endl;
                    return;
                }
            }
        }
    } else if (movementProgress_ > 0.0) {
        // Currently moving to the next room
        movementProgress_ -= 1.0; // Decrement by 1 second per update
        std::cout << "Robot " << name << " is moving to room " << nextRoom_->getRoomName()
                  << ". Time remaining: " << movementProgress_ << " seconds." << std::endl;
        if (movementProgress_ <= 0.0) {
            movementProgress_ = 0.0;
            if (nextRoom_) {
                currentRoom_ = nextRoom_;
                nextRoom_ = nullptr;
                depleteBattery(5.0);
                std::cout << "Robot " << name << " arrived at room " << currentRoom_->getRoomName() << "." << std::endl;

                if (currentRoom_ == targetRoom_) {
                    if (currentCleaningTask_) {
                        // Start cleaning with the specified strategy
                        startCleaning(currentCleaningTask_->getCleanType());
                        // Don't reset currentCleaningTask_ here, we need it to mark completion later
                    } else if (currentRoom_->getRoomId() == 0 && returningToCharger_) {
                        // Start charging
                        startCharging();
                    } else {
                        // Check if there's a task for the current room
                        if (!taskQueue_.empty()) {
                            auto nextTask = taskQueue_.front();
                            if (nextTask->getRoom()->getRoomId() == currentRoom_->getRoomId()) {
                                // Remove task from queue and start cleaning
                                currentCleaningTask_ = nextTask;
                                taskQueue_.pop();
                                startCleaning(currentCleaningTask_->getCleanType());
                                // currentCleaningTask_.reset();
                            }
                        }
                    }
                }
            } else {
                std::cerr << "Robot " << name << ": Error - nextRoom_ is null when movementProgress_ <= 0." << std::endl;
            }
        }
    } else if (!movementQueue_.empty()) {
        // Start moving to the next room
        nextRoom_ = movementQueue_.front();
        movementQueue_.pop();
        movementProgress_ = 5.0; // Movement time in seconds
        std::cout << "Robot " << name << " started moving to room " << nextRoom_->getRoomName() << "." << std::endl;
    } else if (cleaning_) {
        // Continue cleaning
        depleteBattery(1.0); // Deplete battery for cleaning
        depleteWater(1.0);   // Deplete water for cleaning
        cleaningTimeRemaining_ -= 1.0; // Assuming update is called every second
        std::cout << "Robot " << name << " is cleaning room " << currentRoom_->getRoomName()
                  << ". Time remaining: " << cleaningTimeRemaining_ << " seconds." << std::endl;
        if (cleaningTimeRemaining_ <= 0.0) {
            // Cleaning completed
            stopCleaning();
            std::cout << "Robot " << name << " completed cleaning room " << currentRoom_->getRoomName() << "." << std::endl;
            if (currentRoom_) {
                currentRoom_->markClean();
                std::cout << "Room " << currentRoom_->getRoomName() << " marked as clean." << std::endl;
            }
            if (currentCleaningTask_) {
                currentCleaningTask_->markCompleted();
                currentCleaningTask_.reset();
            }
        }
    } else if (!taskQueue_.empty() && !cleaning_ && movementProgress_ <= 0.0 && movementQueue_.empty()) {
        // Only start a new task if the robot is idle
        // Start next task
        auto task = taskQueue_.front();
        taskQueue_.pop();
        currentCleaningTask_ = task;

        Room* targetRoom = task->getRoom();
        std::vector<int> path = map.getRoute(*currentRoom_, *targetRoom);
        if (!path.empty()) {
            setMovementPath(path, map);
            setTargetRoom(targetRoom);
            // Save cleaning task for use after reaching the room
            currentCleaningTask_ = task;
            std::cout << "Robot " << name << " is assigned to clean room " << targetRoom->getRoomName()
                    << " with strategy " << cleaningStrategyToString(task->getCleanType()) << "." << std::endl;
        } else {
            std::cerr << "Robot " << name << ": No path to room " << targetRoom->getRoomId() << std::endl;
        }
    
    } else if (hasPendingTasks_) {
        // No more tasks, return to charger
        Room* chargingStation = map.getRoomById(0);
        if (chargingStation && currentRoom_ != chargingStation) {
            std::vector<int> pathToCharger = map.getRoute(*currentRoom_, *chargingStation);
            if (!pathToCharger.empty()) {
                setMovementPath(pathToCharger, map);
                setTargetRoom(chargingStation);
                hasPendingTasks_ = false;
                std::cout << "Robot " << name << " has completed all tasks and is returning to charger." << std::endl;
            }
        }
    } else {
        // Idle state
        std::cout << "Robot " << name << " is idle in room " << currentRoom_->getRoomName() << "." << std::endl;
    }
}

void Robot::startCharging() {
    isCharging_ = true;
    chargingTimeRemaining_ = 20.0; // Charging takes 20 seconds
}

void Robot::stopCharging() {
    isCharging_ = false;
    chargingTimeRemaining_ = 0.0;
}

bool Robot::isCharging() const {
    return isCharging_;
}

std::string Robot::getStatus() const {
    std::string status = "Normal";
    if (isCharging_) {
        status = "Charging";
    } else if (cleaning_) {
        status = "Cleaning";
    } else if (needsMaintenance()) {
        status = "Needs Maintenance";
    } else if (batteryLevel < 20.0) {
        status = "Low Battery";
    } else if (waterLevel_ < 20.0) {
        status = "Low Water";
    }
    return status;
}

void Robot::addTaskToQueue(const std::shared_ptr<CleaningTask>& task) {
    taskQueue_.push(task);
    hasPendingTasks_ = true; // Set the flag to indicate pending tasks
}

void Robot::startCleaning(CleaningTask::CleanType cleaningType) {
    cleaning_ = true;
    // Set cleaning time and water usage based on cleaning type
    switch (cleaningType) {
        case CleaningTask::VACUUM:
            cleaningTimeRemaining_ = 10.0;
            depleteWater(5.0);
            break;
        case CleaningTask::SCRUB:
            cleaningTimeRemaining_ = 15.0;
            depleteWater(10.0);
            break;
        case CleaningTask::SHAMPOO:
            cleaningTimeRemaining_ = 20.0;
            depleteWater(15.0);
            break;
    }
}

const std::queue<std::shared_ptr<CleaningTask>>& Robot::getTaskQueue() const {
    return taskQueue_;
}
