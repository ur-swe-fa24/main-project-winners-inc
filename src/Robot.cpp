#include "Robot/Robot.h"
#include <iostream>

// Constructor implementation
Robot::Robot(const std::string& name, int batteryLevel)
    : name(name), batteryLevel(batteryLevel), cleaning_(false), lowBatteryAlertSent_(false) {}

// New methods
void Robot::startCleaning() {
    cleaning_ = true;
}

void Robot::stopCleaning() {
    cleaning_ = false;
}

bool Robot::isCleaning() const {
    return cleaning_;
}

void Robot::setLowBatteryAlertSent(bool sent) {
    lowBatteryAlertSent_ = sent;
}

bool Robot::isLowBatteryAlertSent() const {
    return lowBatteryAlertSent_;
}

// Method to send a status update
void Robot::sendStatusUpdate() const {
    std::cout << "Robot " << name << " has " << batteryLevel << "% battery remaining." << std::endl;
}

// Method to recharge the robot's battery
void Robot::recharge() {
    batteryLevel = 100;  // Set battery level to full
    std::cout << "Robot " << name << " is fully recharged." << std::endl;
}

// Method to check if the robot needs maintenance (e.g., if battery is below a threshold)
bool Robot::needsMaintenance() const {
    if (batteryLevel < 20) {
        std::cout << "Robot " << name << " needs maintenance (battery level is low)." << std::endl;
        return true;
    }
    return false;
}

// Getter for robot name
std::string Robot::getName() const {
    return name;
}

// Getter for battery level
int Robot::getBatteryLevel() const {
    return batteryLevel;
}

// Method to deplete battery by a specified amount (for testing purposes)
void Robot::depleteBattery(int amount) {
    batteryLevel -= amount;
    if (batteryLevel < 0) batteryLevel = 0;  // Prevent battery from going negative
    std::cout << "Robot " << name << "'s battery depleted to " << batteryLevel << "%." << std::endl;
}
