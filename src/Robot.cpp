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

// Status and Updates
void Robot::sendStatusUpdate() const {
    std::cout << getStatus() << std::endl;
}

std::string Robot::getStatus() const {
    std::string status = "Robot " + name + " Status:\n";
    status += "Battery Level: " + std::to_string(batteryLevel) + "%\n";
    status += "Water Level: " + std::to_string(waterLevel_) + "%\n";
    status += "Current State: ";
    if (isCharging_) status += "Charging\n";
    else if (cleaning_) status += "Cleaning\n";
    else if (isMoving()) status += "Moving\n";
    else status += "Idle\n";
    return status;
}

void Robot::update(const Map& map) {
    const double deltaTime = 0.1; // 100ms update interval
    
    updateBatteryStatus(deltaTime);
    updateWaterStatus(deltaTime);
    updateMovement(deltaTime);
    updateCleaning(deltaTime);
    updateTaskProgress(deltaTime);
    handleLowResources();
}

// Battery Management
void Robot::updateBatteryStatus(double deltaTime) {
    if (isCharging_) {
        batteryLevel = std::min(100.0, batteryLevel + (20.0 * deltaTime)); // Charge at 20% per second
        if (batteryLevel >= 100.0) {
            stopCharging();
        }
    } else {
        // Deplete battery based on current activity
        double depletion = 1.0 * deltaTime; // Base depletion
        if (cleaning_) depletion += 2.0 * deltaTime;
        if (isMoving()) depletion += 1.5 * deltaTime;
        depleteBattery(depletion);
    }
}

void Robot::depleteBattery(double amount) {
    batteryLevel = std::max(0.0, batteryLevel - amount);
    if (batteryLevel < 20.0 && !lowBatteryAlertSent_) {
        std::cout << "Warning: " << name << " battery level low (" << batteryLevel << "%)" << std::endl;
        lowBatteryAlertSent_ = true;
    }
}

bool Robot::needsCharging() const {
    return batteryLevel < 20.0;
}

void Robot::startCharging() {
    isCharging_ = true;
    chargingTimeRemaining_ = (100.0 - batteryLevel) * 0.05; // 5 seconds per 1% charge
}

void Robot::stopCharging() {
    isCharging_ = false;
    chargingTimeRemaining_ = 0.0;
}

bool Robot::isCharging() const {
    return isCharging_;
}

// Water Management
void Robot::updateWaterStatus(double deltaTime) {
    if (cleaning_) {
        double depletion = 5.0 * deltaTime; // Base water depletion while cleaning
        if (currentRoom_ && currentRoom_->flooringType == "Carpet") {
            depletion *= 1.5; // Carpet cleaning uses more water
        }
        depleteWater(depletion);
    }
}

void Robot::depleteWater(double amount) {
    waterLevel_ = std::max(0.0, waterLevel_ - amount);
    if (waterLevel_ < 20.0 && !lowWaterAlertSent_) {
        std::cout << "Warning: " << name << " water level low (" << waterLevel_ << "%)" << std::endl;
        lowWaterAlertSent_ = true;
    }
}

// Movement and Navigation
void Robot::updateMovement(double deltaTime) {
    if (!isMoving() || isCharging_) return;
    
    movementProgress_ += deltaTime * 10.0; // Move at 10% per second
    if (movementProgress_ >= 100.0) {
        currentRoom_ = nextRoom_;
        nextRoom_ = nullptr;
        movementProgress_ = 0.0;
        
        if (currentRoom_ == targetRoom_) {
            if (returningToCharger_) {
                startCharging();
                returningToCharger_ = false;
            } else if (hasPendingTasks_) {
                resumeSavedTask();
            }
        }
    }
}

bool Robot::isMoving() const {
    return nextRoom_ != nullptr;
}

// Cleaning Operations
void Robot::updateCleaning(double deltaTime) {
    if (!cleaning_ || waterLevel_ < 5.0 || batteryLevel < 5.0) {
        stopCleaning();
        return;
    }
    
    cleaningTimeRemaining_ -= deltaTime;
    if (cleaningTimeRemaining_ <= 0.0) {
        stopCleaning();
        if (!taskQueue_.empty()) {
            taskQueue_.pop();
        }
    }
}

void Robot::startCleaning() {
    if (waterLevel_ < 10.0) {
        std::cout << "Warning: Water level too low to start cleaning!" << std::endl;
        return;
    }
    
    cleaning_ = true;
    if (currentRoom_) {
        if (currentRoom_->flooringType == "Hardwood") {
            cleaningTimeRemaining_ = 15.0;
            depleteWater(5.0);
        } else if (currentRoom_->flooringType == "Carpet") {
            cleaningTimeRemaining_ = 20.0;
            depleteWater(10.0);
        } else {
            cleaningTimeRemaining_ = 10.0;
            depleteWater(7.0);
        }
    }
}

void Robot::stopCleaning() {
    cleaning_ = false;
    cleaningTimeRemaining_ = 0.0;
}

void Robot::startCleaning(CleaningTask::CleanType cleaningType) {
    if (batteryLevel < 20.0 || waterLevel_ < 20.0) {
        std::cout << "Cannot start cleaning: insufficient resources" << std::endl;
        return;
    }
    
    cleaning_ = true;
    cleaningTimeRemaining_ = 60.0; // Set default cleaning time to 60 seconds
}

// Task Management
void Robot::updateTaskProgress(double deltaTime) {
    if (!cleaning_ && !taskQueue_.empty() && !isMoving() && !isCharging_) {
        auto& task = taskQueue_.front();
        if (task->getRoom() != currentRoom_) {
            setTargetRoom(task->getRoom());
        } else {
            startCleaning(task->getCleanType());
        }
    }
}

void Robot::saveCurrentTask() {
    if (!taskQueue_.empty()) {
        savedTask_ = taskQueue_.front();
        taskQueue_.pop();
        hasPendingTasks_ = true;
    }
}

void Robot::resumeSavedTask() {
    if (savedTask_) {
        taskQueue_.push(savedTask_);
        savedTask_ = nullptr;
        hasPendingTasks_ = false;
    }
}

void Robot::handleLowResources() {
    if (needsCharging() || needsWaterRefill()) {
        if (!returningToCharger_ && !isCharging_) {
            saveCurrentTask();
            returningToCharger_ = true;
            if (currentRoom_ && currentRoom_->getMap()) {
                recharge(*(currentRoom_->getMap()));
            }
        }
    }
}

// Utility functions
bool Robot::moveToRoom(Room* room) {
    if (!room) return false;
    
    // Set the target room and update current room
    setTargetRoom(room);
    setCurrentRoom(room);
    
    // Reset movement progress
    movementProgress_ = 0.0;
    
    return true;
}

std::string Robot::getName() const {
    return name;
}

std::string Robot::cleaningStrategyToString(CleaningTask::CleanType cleanType) {
    switch (cleanType) {
        case CleaningTask::VACUUM:
            return "Vacuum";
        case CleaningTask::SCRUB:
            return "Scrub";
        case CleaningTask::SHAMPOO:
            return "Shampoo";
        default:
            return "Unknown";
    }
}

double Robot::getBatteryLevel() const {
    return batteryLevel;
}

double Robot::getWaterLevel() const {
    return waterLevel_;
}

Room* Robot::getTargetRoom() const {
    return targetRoom_;
}

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

void Robot::setTargetRoom(Room* room) {
    targetRoom_ = room;
}

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

void Robot::addTaskToQueue(const std::shared_ptr<CleaningTask>& task) {
    taskQueue_.push(task);
    hasPendingTasks_ = true; // Set the flag to indicate pending tasks
}

const std::queue<std::shared_ptr<CleaningTask>>& Robot::getTaskQueue() const {
    return taskQueue_;
}

bool Robot::needsMaintenance() const {
    if (batteryLevel < 20.0) {
        std::cout << "Robot " << name << " needs maintenance (battery level is low)." << std::endl;
        return true;
    }
    return false;
}

bool Robot::needsWaterRefill() const {
    return waterLevel_ < 20.0;
}

void Robot::refillWater() {
    waterLevel_ = 100.0;
    lowWaterAlertSent_ = false;
    std::cout << "Robot " << name << " water tank refilled to 100%" << std::endl;
}

void Robot::setLowBatteryAlertSent(bool sent) {
    lowBatteryAlertSent_ = sent;
}

bool Robot::isLowBatteryAlertSent() const {
    return lowBatteryAlertSent_;
}

void Robot::setLowWaterAlertSent(bool sent) {
    lowWaterAlertSent_ = sent;
}

bool Robot::isLowWaterAlertSent() const {
    return lowWaterAlertSent_;
}

bool Robot::isCleaning() const {
    return cleaning_;
}

bool Robot::usesWater() const{
    return waterLevel_ > 0.0;
}
