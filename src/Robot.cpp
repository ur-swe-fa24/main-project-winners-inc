#include "Robot/Robot.h"
#include "map/map.h"
#include <iostream>

// Constructor implementation
Robot::Robot(const std::string& name, int batteryLevel)
    : name(name), batteryLevel(batteryLevel), cleaning_(false), lowBatteryAlertSent_(false), currentRoom_(nullptr) {}

// Methods to start and stop cleaning
void Robot::startCleaning() {
    cleaning_ = true;
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

// Send status update
void Robot::sendStatusUpdate() const {
    std::cout << "Robot " << name << " has " << batteryLevel << "% battery remaining." << std::endl;
}

// Recharge battery
void Robot::recharge() {
    batteryLevel = 100;
    std::cout << "Robot " << name << " is fully recharged." << std::endl;
}

// Check if robot needs maintenance
bool Robot::needsMaintenance() const {
    if (batteryLevel < 20) {
        std::cout << "Robot " << name << " needs maintenance (battery level is low)." << std::endl;
        return true;
    }
    return false;
}

// Getters
std::string Robot::getName() const {
    return name;
}

int Robot::getBatteryLevel() const {
    return batteryLevel;
}

// Deplete battery
void Robot::depleteBattery(int amount) {
    batteryLevel -= amount;
    if (batteryLevel < 0) batteryLevel = 0;
    std::cout << "Robot " << name << "'s battery depleted to " << batteryLevel << "%." << std::endl;
}

// Current room management
void Robot::setCurrentRoom(Room* room) {
    currentRoom_ = room;
}

Room* Robot::getCurrentRoom() const {
    return currentRoom_;
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
void Robot::setMovementPath(const std::vector<int>& roomIds, Map& map) {
    while (!movementQueue_.empty()) {
        movementQueue_.pop();
    }
    for (int roomId : roomIds) {
        Room* room = map.getRoomById(roomId);
        if (room) {
            movementQueue_.push(room);
        }
    }
}

// Update robot status
void Robot::update() {
    if (!movementQueue_.empty()) {
        Room* nextRoom = movementQueue_.front();
        if (moveToRoom(nextRoom)) {
            movementQueue_.pop();
            depleteBattery(5);  // Deplete battery for movement
        }
    } else if (isCleaning()) {
        depleteBattery(1);  // Deplete battery for cleaning
        // Optionally perform cleaning actions
    }
}
