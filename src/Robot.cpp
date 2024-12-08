#include "Robot/Robot.h"
#include "CleaningTask/cleaningTask.h"
#include "Room/Room.h"
#include "map/map.h"
#include <algorithm>
#include <iostream>

Robot::Robot(const std::string& name, double batteryLevel, double waterLevel)
    : name_(name), batteryLevel_(batteryLevel), waterLevel_(waterLevel),
      cleaning_(false), isCharging_(false), cleaningProgress_(0.0), movementProgress_(0.0),
      currentRoom_(nullptr), nextRoom_(nullptr), cleaningTimeRemaining_(0.0),
      targetRoom_(nullptr), lowBatteryAlertSent_(false), lowWaterAlertSent_(false),
      currentTask_(nullptr)
{}

void Robot::updateState(double deltaTime) {
    if (isCharging_) {
        batteryLevel_ = std::min(100.0, batteryLevel_ + deltaTime * 20.0);
    } else {
        if (cleaning_ && currentTask_) {
            double batteryDepletion = 2.0 * deltaTime;
            batteryLevel_ = std::max(0.0, batteryLevel_ - batteryDepletion);

            if (currentTask_->getCleanType() == CleaningTask::SHAMPOO) {
                double waterDepletion = 1.0 * deltaTime;
                waterLevel_ = std::max(0.0, waterLevel_ - waterDepletion);
            }
        }
    }

    // Handle movement
    if (isMoving()) {
        movementProgress_ += deltaTime * 10.0;
        if (movementProgress_ >= 100.0) {
            currentRoom_ = nextRoom_;
            nextRoom_ = nullptr;
            movementProgress_ = 0.0;
            // If there are more steps in movementQueue_, move on
            if (!movementQueue_.empty()) {
                nextRoom_ = movementQueue_.front();
                movementQueue_.pop();
            } else {
                // Movement finished, check if we're at targetRoom_ and have a task
                if (currentTask_ && targetRoom_ == currentRoom_ && !cleaning_) {
                    startCleaning(currentTask_->getCleanType());
                }
            }
        }
    }

    // Handle cleaning time
    if (cleaning_ && currentTask_) {
        cleaningTimeRemaining_ -= deltaTime;
        if (cleaningTimeRemaining_ <= 0) {
            cleaning_ = false;
            currentTask_->markCompleted();
            currentTask_.reset();
            cleaningProgress_ = 0.0;
            if (currentRoom_) {
                currentRoom_->markClean();
            }
        }
    }

    if (needsCharging() && !lowBatteryAlertSent_) {
        lowBatteryAlertSent_ = true;
    }
    if (needsWaterRefill() && !lowWaterAlertSent_) {
        lowWaterAlertSent_ = true;
    }
}

void Robot::startCleaning(CleaningTask::CleanType cleaningType) {
    if (isCleaning() || !currentRoom_ || !currentTask_) return;
    cleaning_ = true;

    double baseTime = 15.0; // default medium
    std::string size = currentRoom_->getSize();
    std::transform(size.begin(), size.end(), size.begin(), ::tolower);

    if (size == "small") {
        baseTime = 5.0;
    } else if (size == "medium") {
        baseTime = 10.0;
    } else if (size == "large") {
        baseTime = 15.0;
    }

    cleaningTimeRemaining_ = baseTime;
}

void Robot::stopCleaning() {
    cleaning_ = false;
    currentTask_.reset();
    cleaningProgress_ = 0.0;
}

void Robot::setMovementPath(const std::vector<int>& roomIds, const Map& map) {
    while (!movementQueue_.empty()) movementQueue_.pop();

    for (int roomId : roomIds) {
        Room* r = map.getRoomById(roomId);
        if (r) {
            // Don't skip current room
            movementQueue_.push(r);
        }
    }

    // Pop the first room if it equals currentRoom_ to avoid redundant step
    if (!movementQueue_.empty() && movementQueue_.front() == currentRoom_) {
        movementQueue_.pop();
    }

    if (!movementQueue_.empty()) {
        nextRoom_ = movementQueue_.front();
        movementQueue_.pop();
    } else {
        nextRoom_ = nullptr;
    }
}

void Robot::moveToRoom(Room* room) {
    currentRoom_ = room;
    movementProgress_ = 0.0;
    nextRoom_ = nullptr;
    while(!movementQueue_.empty()) movementQueue_.pop();
}

Room* Robot::getNextRoom() const {
    return nextRoom_;
}

void Robot::setTargetRoom(Room* room) {
    targetRoom_ = room;
}


double Robot::getBatteryLevel() const { return batteryLevel_; }
double Robot::getWaterLevel() const { return waterLevel_; }
bool Robot::needsCharging() const { return batteryLevel_ < 20.0; }
bool Robot::needsWaterRefill() const { return waterLevel_ < 20.0; }
bool Robot::isCleaning() const { return cleaning_; }
bool Robot::isMoving() const { return nextRoom_ != nullptr; }
Room* Robot::getCurrentRoom() const { return currentRoom_; }
std::string Robot::getName() const { return name_; }
void Robot::setCurrentRoom(Room* room) { currentRoom_ = room; }
void Robot::setCharging(bool charging) { isCharging_ = charging; }
void Robot::refillWater() { waterLevel_ = 100.0; }
void Robot::fullyRecharge() { batteryLevel_ = 100.0; }

bool Robot::isCharging() const { return isCharging_; }
double Robot::getMovementProgress() const { return movementProgress_; }
bool Robot::needsMaintenance() const { return false; }
bool Robot::isLowBatteryAlertSent() const { return lowBatteryAlertSent_; }
bool Robot::isLowWaterAlertSent() const { return lowWaterAlertSent_; }

std::string Robot::getStatus() const {
    if (isCharging_) return "Charging";
    if (cleaning_) return "Cleaning";
    if (isMoving()) return "Moving";
    return "Idle";
}

void Robot::setLowBatteryAlertSent(bool val) { lowBatteryAlertSent_ = val; }
void Robot::setLowWaterAlertSent(bool val) { lowWaterAlertSent_ = val; }
void Robot::setCurrentTask(std::shared_ptr<CleaningTask> task) {
    currentTask_ = task;
}
