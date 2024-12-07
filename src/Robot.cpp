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
      currentTask_(nullptr) // Initialize to null
{}

void Robot::updateState(double deltaTime) {
    // Battery/water logic (simplified)
    if (isCharging_) {
        batteryLevel_ = std::min(100.0, batteryLevel_ + deltaTime * 20.0);
    } else {
        double depletion = deltaTime;
        if (cleaning_) depletion += 2.0 * deltaTime;
        batteryLevel_ = std::max(0.0, batteryLevel_ - depletion);
    }

    // Movement logic
    if (isMoving()) {
        movementProgress_ += deltaTime * 10.0; // Move 10% per second
        if (movementProgress_ >= 100.0) {
            currentRoom_ = nextRoom_;
            nextRoom_ = nullptr;
            movementProgress_ = 0.0;
        }
    }

    // Cleaning logic
    if (cleaning_ && currentTask_) {
        cleaningTimeRemaining_ -= deltaTime;
        if (cleaningTimeRemaining_ <= 0) {
            cleaning_ = false;
            currentTask_->markCompleted(); // Now works because currentTask_ is CleaningTask
            currentTask_.reset();          // Reset the shared_ptr
            cleaningProgress_ = 0.0;
            if (currentRoom_) {
                currentRoom_->markClean();
            }
        }
    }

    // Alerts
    if (needsCharging() && !lowBatteryAlertSent_) {
        lowBatteryAlertSent_ = true;
    }
    if (needsWaterRefill() && !lowWaterAlertSent_) {
        lowWaterAlertSent_ = true;
    }
}

void Robot::startCleaning(CleaningTask::CleanType cleaningType) {
    if (isCleaning() || !currentRoom_) return;
    cleaning_ = true;
    cleaningTimeRemaining_ = 60.0; // Example cleaning duration
    // Could set cleaningTimeRemaining_ based on room/floor type
}

void Robot::stopCleaning() {
    cleaning_ = false;
    currentTask_.reset();
    cleaningProgress_ = 0.0;
}

void Robot::setMovementPath(const std::vector<int>& roomIds, const Map& map) {
    // Clear any existing movement queue
    while (!movementQueue_.empty()) movementQueue_.pop();

    // Populate movement queue with room pointers
    for (size_t i = 0; i < roomIds.size(); ++i) {
        Room* r = map.getRoomById(roomIds[i]);
        if (r) {
            // Skip first if it's current room
            if (i == 0 && r == currentRoom_) continue;
            movementQueue_.push(r);
        }
    }

    // Set nextRoom_ from queue
    if (!movementQueue_.empty()) {
        nextRoom_ = movementQueue_.front();
        movementQueue_.pop();
    }
}

void Robot::moveToRoom(Room* room) {
    // Direct move
    currentRoom_ = room;
    movementProgress_ = 0.0;
    nextRoom_ = nullptr;
    while(!movementQueue_.empty()) movementQueue_.pop();
}
Room* Robot::getNextRoom() const {
    return nextRoom_;
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
