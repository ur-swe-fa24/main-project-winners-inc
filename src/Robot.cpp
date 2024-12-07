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

    // Handle charging
    if (isCharging_) {
        batteryLevel = std::min(100.0, batteryLevel + (20.0 * deltaTime)); // 20% per second
        if (batteryLevel >= 100.0) {
            std::cout << "Robot " << name << " finished charging" << std::endl;
            stopCharging();
            // Resume saved task if any
            if (!taskQueue_.empty()) {
                auto task = taskQueue_.front();
                if (task->getRoom()) {
                    moveToRoom(task->getRoom());
                }
            }
        }
        return; // Don't do anything else while charging
    }

    // Handle movement
    if (isMoving()) {
        updateMovement(deltaTime);
        // Don't clean while moving
        return;
    }

    // Handle cleaning
    if (cleaning_) {
        updateCleaning(deltaTime);
        return;
    }

    // If idle and has tasks, process them
    if (!taskQueue_.empty() && !isMoving() && !cleaning_ && !isCharging_) {
        auto task = taskQueue_.front();
        if (task->getRoom()) {
            if (task->getRoom() == currentRoom_) {
                // We're in the right room, start cleaning
                startCleaning(task->getCleanType());
                taskQueue_.pop();
            } else {
                // Move to the target room
                moveToRoom(task->getRoom());
            }
        }
    }

    // If battery is low and not charging, return to charger
    if (needsCharging() && !isCharging_ && !returningToCharger_) {
        Room* charger = map.getRoomById(0); // Assuming room 0 is charger
        if (charger && currentRoom_ != charger) {
            std::cout << "Robot " << name << " returning to charger" << std::endl;
            returningToCharger_ = true;
            moveToRoom(charger);
        } else if (charger && currentRoom_ == charger) {
            // We're at the charger, start charging
            startCharging();
        }
    }
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
    if (batteryLevel < 10.0 && !lowBatteryAlertSent_) {
        std::cout << "Warning: " << name << " battery level low (" << batteryLevel << "%)" << std::endl;
        lowBatteryAlertSent_ = true;
    }
}

bool Robot::needsCharging() const {
    return batteryLevel < 10.0;
}

void Robot::startCharging() {
    if (!isCharging_) {
        isCharging_ = true;
        returningToCharger_ = false;  // Reset returning flag when charging starts
        chargingTimeRemaining_ = (100.0 - batteryLevel) * 0.05; // 5 seconds per 1% charge
        std::cout << "Robot " << name << " started charging at " << batteryLevel << "%" << std::endl;
    }
}

void Robot::stopCharging() {
    if (isCharging_) {
        isCharging_ = false;
        chargingTimeRemaining_ = 0.0;
        lowBatteryAlertSent_ = false;  // Reset low battery alert
        std::cout << "Robot " << name << " stopped charging at " << batteryLevel << "%" << std::endl;
    }
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
bool Robot::moveToRoom(Room* room) {
    if (!room) {
        std::cout << "Error: Cannot move to null room" << std::endl;
        return false;
    }

    if (room == currentRoom_) {
        std::cout << "Robot " << name << " is already in " << room->getRoomName() << std::endl;
        return true;
    }

    // Stop cleaning if we're moving
    if (cleaning_) {
        stopCleaning();
    }

    // Check if we're already moving
    if (isMoving()) {
        std::cout << "Robot " << name << " is already moving" << std::endl;
        return false;
    }

    nextRoom_ = room;
    // Set movement time to 2 seconds (more reasonable for simulation)
    movementProgress_ = 2.0;
    std::cout << "Robot " << name << " moving from " 
              << (currentRoom_ ? currentRoom_->getRoomName() : "Unknown")
              << " to " << room->getRoomName() << std::endl;
    return true;
}

void Robot::updateMovement(double deltaTime) {
    if (!isMoving() || isCharging_) return;

    // Validate next room is still valid
    if (!nextRoom_) {
        std::cout << "Robot " << name << " movement error: next room is null" << std::endl;
        movementProgress_ = 0.0;
        return;
    }

    // Deplete battery during movement (reduced from 1.5 to 0.5)
    depleteBattery(deltaTime * 0.5); // Moving uses less battery now

    if (needsCharging()) {
        std::cout << "Robot " << name << " needs charging, stopping movement" << std::endl;
        nextRoom_ = nullptr;
        movementProgress_ = 0.0;
        return;
    }

    movementProgress_ -= deltaTime;
    if (movementProgress_ <= 0.0) {
        // Movement complete
        currentRoom_ = nextRoom_;
        nextRoom_ = nullptr;
        movementProgress_ = 0.0;
        std::cout << "Robot " << name << " arrived at " << currentRoom_->getRoomName() << std::endl;
        
        // Check if there are more rooms in the queue
        if (!movementQueue_.empty()) {
            Room* nextRoomInQueue = movementQueue_.front();
            movementQueue_.pop();
            if (nextRoomInQueue) {
                moveToRoom(nextRoomInQueue);
            } else {
                std::cout << "Robot " << name << " skipping invalid room in queue" << std::endl;
            }
        } else if (currentRoom_ == targetRoom_ && !cleaning_) {
            // If this was the target room, start cleaning
            startCleaning();
        }
    } else {
        // Update progress percentage for UI feedback
        double progressPercent = 100.0 * (1.0 - (movementProgress_ / 2.0));
        if (static_cast<int>(progressPercent * 10) % 100 == 0) { // Log every 10%
            std::cout << "Robot " << name << " movement progress: " << progressPercent << "%" << std::endl;
        }
    }
}

bool Robot::isMoving() const {
    return nextRoom_ != nullptr;
}

// Cleaning Operations
void Robot::updateCleaning(double deltaTime) {
    if (!cleaning_ || isCharging_ || isMoving()) return;

    // Check resources
    if (needsCharging()) {
        std::cout << "Robot " << name << " needs charging, stopping cleaning" << std::endl;
        stopCleaning();
        return;
    }

    if (needsWaterRefill()) {
        std::cout << "Robot " << name << " needs water refill, stopping cleaning" << std::endl;
        stopCleaning();
        return;
    }

    // Update cleaning progress
    cleaningTimeRemaining_ -= deltaTime;

    // Deplete resources
    depleteBattery(deltaTime * 2.0); // Cleaning uses more battery
    if (currentRoom_) {
        double waterUsage = deltaTime * (currentRoom_->flooringType == "Carpet" ? 3.0 : 2.0);
        depleteWater(waterUsage);
    }

    // Check if cleaning is complete
    if (cleaningTimeRemaining_ <= 0.0) {
        std::cout << "Robot " << name << " finished cleaning " 
                  << (currentRoom_ ? currentRoom_->getRoomName() : "Unknown") << std::endl;
        stopCleaning();

        // If we have more tasks, process them
        if (!taskQueue_.empty()) {
            auto nextTask = taskQueue_.front();
            taskQueue_.pop();
            if (nextTask->getRoom()) {
                moveToRoom(nextTask->getRoom());
            }
        }
    }
}

void Robot::startCleaning() {
    if (!currentRoom_) {
        std::cout << "Error: Robot is not in any room" << std::endl;
        return;
    }

    if (isMoving()) {
        std::cout << "Error: Cannot start cleaning while moving" << std::endl;
        return;
    }

    if (isCharging()) {
        std::cout << "Error: Cannot start cleaning while charging" << std::endl;
        return;
    }

    if (waterLevel_ < 10.0) {
        std::cout << "Warning: Water level too low to start cleaning!" << std::endl;
        return;
    }

    if (batteryLevel < 20.0) {
        std::cout << "Warning: Battery level too low to start cleaning!" << std::endl;
        return;
    }

    cleaning_ = true;
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

    std::cout << "Robot " << name << " started cleaning " << currentRoom_->getRoomName() << std::endl;
}

void Robot::stopCleaning() {
    if (cleaning_) {
        std::cout << "Robot " << name << " stopped cleaning" << std::endl;
        cleaning_ = false;
        cleaningTimeRemaining_ = 0.0;
    }
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
void Robot::addTaskToQueue(const std::shared_ptr<CleaningTask>& task) {
    if (!task) {
        std::cout << "Error: Cannot add null task to queue" << std::endl;
        return;
    }

    if (!task->getRoom()) {
        std::cout << "Error: Task has no target room" << std::endl;
        return;
    }

    taskQueue_.push(task);
    std::cout << "Added cleaning task for room " << task->getRoom()->getRoomName() 
              << " to robot " << name << "'s queue" << std::endl;

    // If robot is idle, start processing the task
    if (!isMoving() && !cleaning_ && !isCharging_) {
        if (task->getRoom() == currentRoom_) {
            startCleaning(task->getCleanType());
            taskQueue_.pop();
        } else {
            moveToRoom(task->getRoom());
        }
    }
}

void Robot::saveCurrentTask() {
    if (cleaning_ && currentRoom_) {
        // Create a new task for the current cleaning operation
        auto task = std::make_shared<CleaningTask>(currentRoom_, CleaningTask::CleanType::VACUUM);
        taskQueue_.push(task);
        std::cout << "Saved current cleaning task for room " << currentRoom_->getRoomName() << std::endl;
    }
}

void Robot::resumeSavedTask() {
    if (!taskQueue_.empty()) {
        auto task = taskQueue_.front();
        if (task->getRoom() == currentRoom_) {
            startCleaning(task->getCleanType());
            taskQueue_.pop();
        } else {
            moveToRoom(task->getRoom());
        }
        std::cout << "Resuming saved task for room " << task->getRoom()->getRoomName() << std::endl;
    }
}

void Robot::clearTaskQueue() {
    std::queue<std::shared_ptr<CleaningTask>> empty;
    std::swap(taskQueue_, empty);
    std::cout << "Cleared task queue for robot " << name << std::endl;
}

bool Robot::hasPendingTasks() const {
    return !taskQueue_.empty();
}

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

    // Start movement to first room in queue if queue is not empty
    if (!movementQueue_.empty()) {
        Room* nextRoomInQueue = movementQueue_.front();
        movementQueue_.pop();
        if (nextRoomInQueue) {
            moveToRoom(nextRoomInQueue);
        }
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
