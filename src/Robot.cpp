#include "Robot/Robot.h"
#include "CleaningTask/cleaningTask.h"
#include "Room/Room.h"
#include <algorithm>
#include <iostream>

Robot::Robot(const std::string& name, double batteryLevel, double waterLevel)
    : name_(name), batteryLevel_(batteryLevel), waterLevel_(waterLevel),
      cleaning_(false), isCharging_(false), cleaningProgress_(0.0), movementProgress_(0.0),
      currentRoom_(nullptr), currentTask_(nullptr),
      lowBatteryAlertSent_(false), lowWaterAlertSent_(false) {}

void Robot::updateState(double deltaTime) {
    if (isCharging_) {
        batteryLevel_ = std::min(100.0, batteryLevel_ + deltaTime * 20.0);
        if (batteryLevel_ == 100.0) {
            // Fully charged, stop charging
            isCharging_ = false;
        }
    } else {
        double depletion = deltaTime;
        if (cleaning_) depletion += 2.0 * deltaTime;
        batteryLevel_ = std::max(0.0, batteryLevel_ - depletion);
    }

    if (cleaning_ && currentTask_) {
        cleaningProgress_ += deltaTime;
        if (cleaningProgress_ >= 60.0) {
            cleaning_ = false;
            currentTask_->markCompleted();
            currentTask_.reset();
            cleaningProgress_ = 0.0;
            if (currentRoom_) {
                currentRoom_->markClean();
            }
        }
    }

    // Check alerts
    if (needsCharging() && !lowBatteryAlertSent_) {
        lowBatteryAlertSent_ = true;
    }
    if (needsWaterRefill() && !lowWaterAlertSent_) {
        lowWaterAlertSent_ = true;
    }
}

void Robot::startCleaningTask(std::shared_ptr<CleaningTask> task) {
    currentTask_ = task;
    cleaning_ = true;
    cleaningProgress_ = 0.0;
}

void Robot::stopCleaningTask() {
    cleaning_ = false;
    currentTask_.reset();
    cleaningProgress_ = 0.0;
}

void Robot::moveToRoom(Room* targetRoom) {
    // Assume instant move or add pathfinding logic here
    currentRoom_ = targetRoom;
    movementProgress_ = 0.0;
}

double Robot::getBatteryLevel() const { return batteryLevel_; }
double Robot::getWaterLevel() const { return waterLevel_; }
bool Robot::needsCharging() const { return batteryLevel_ < 20.0; }
bool Robot::needsWaterRefill() const { return waterLevel_ < 20.0; }

bool Robot::isCleaning() const { return cleaning_; }
bool Robot::isMoving() const { return movementProgress_ > 0.0; }

Room* Robot::getCurrentRoom() const { return currentRoom_; }
std::string Robot::getName() const { return name_; }

void Robot::setCurrentRoom(Room* room) { currentRoom_ = room; }
void Robot::setCharging(bool charging) { isCharging_ = charging; }
void Robot::refillWater() { waterLevel_ = 100.0; }
void Robot::fullyRecharge() { batteryLevel_ = 100.0; }

// Added methods
bool Robot::isCharging() const {
    return isCharging_;
}

double Robot::getMovementProgress() const {
    return movementProgress_;
}

bool Robot::needsMaintenance() const {
    // Implement logic if needed. For now, always false.
    return false;
}

bool Robot::isLowBatteryAlertSent() const {
    return lowBatteryAlertSent_;
}

bool Robot::isLowWaterAlertSent() const {
    return lowWaterAlertSent_;
}

std::string Robot::getStatus() const {
    if (isCharging_) return "Charging";
    if (cleaning_) return "Cleaning";
    if (isMoving()) return "Moving";
    return "Idle";
}

void Robot::setLowBatteryAlertSent(bool val) {
    lowBatteryAlertSent_ = val;
}

void Robot::setLowWaterAlertSent(bool val) {
    lowWaterAlertSent_ = val;
}
