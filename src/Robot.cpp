#include "Robot/Robot.h"
#include "map/map.h"
#include <iostream>

// Constructor implementation
Robot::Robot(const std::string& name, int batteryLevel, int waterLevel)
    : name(name), batteryLevel(batteryLevel), waterLevel_(waterLevel), cleaning_(false), 
    lowBatteryAlertSent_(false), lowWaterAlertSent_(false), currentRoom_(nullptr), 
    nextRoom_(nullptr), targetRoom_(nullptr), isCharging_(false),
    chargingTimeRemaining_(0), movementProgress_(0.0), cleaningTimeRemaining_(0.0) {}

// Methods to start and stop cleaning
void Robot::startCleaning() {
    if (waterLevel_ < 10) {
        std::cout << "Warning: Water level too low to start cleaning!" << std::endl;
        return;
    }
    
    cleaning_ = true;
    if (currentRoom_) {
        // Adjust cleaning time based on room type
        if (currentRoom_->flooringType == "Hardwood") {
            cleaningTimeRemaining_ = 15.0;
            depleteWater(5); // Hardwood floors use less water
        } else if (currentRoom_->flooringType == "Carpet") {
            cleaningTimeRemaining_ = 20.0;
            depleteWater(10); // Carpet cleaning uses more water
        } else {
            cleaningTimeRemaining_ = 10.0;
            depleteWater(7); // Default water usage
        }
    } else {
        std::cerr << "Error: currentRoom_ is null in startCleaning()" << std::endl;
        cleaningTimeRemaining_ = 10.0;
        depleteWater(7);
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
    waterLevel_ = 100;
    lowWaterAlertSent_ = false;
    std::cout << "Robot " << name << " water tank refilled to 100%" << std::endl;
}

void Robot::depleteWater(int amount) {
    waterLevel_ = std::max(0, waterLevel_ - amount);
    if (waterLevel_ < 20 && !lowWaterAlertSent_) {
        std::cout << "Warning: " << name << " water level low (" << waterLevel_ << "%)" << std::endl;
        lowWaterAlertSent_ = true;
    }
}

int Robot::getWaterLevel() const {
    return waterLevel_;
}

bool Robot::needsWaterRefill() const {
    return waterLevel_ < 20;
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

int Robot::getMovementProgress() const {
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
        if (batteryLevel < 100) {
            batteryLevel = std::min(100, batteryLevel + 10);
        }
        if (waterLevel_ < 100) {
            refillWater();
        }
        return;
    }
    
    std::cout << "Robot " << name << " update called. Current status: " << getStatus() << std::endl;
    if (movementProgress_ > 0.0) {
        // Currently moving to the next room
        movementProgress_ -= 1.0; // Decrement by 1 second per update
        std::cout << "Robot " << name << ": Moving to next room. Remaining time: " << movementProgress_ << " seconds." << std::endl;
        if (movementProgress_ <= 0.0) {
            movementProgress_ = 0.0;
            if (nextRoom_) {
                currentRoom_ = nextRoom_;
                nextRoom_ = nullptr;
                depleteBattery(5);
                std::cout << "Robot " << name << " arrived at room " << currentRoom_->getRoomId() << " (" << currentRoom_->getRoomName() << ")." << std::endl;

                if (currentRoom_ == targetRoom_) {
                    if (currentRoom_->getRoomId() == 0) {
                        // If at charging station, start charging
                        startCharging();
                        std::cout << "Robot " << name << " started charging." << std::endl;
                    } else {
                        // Start cleaning
                        startCleaning();
                        std::cout << "Robot " << name << " started cleaning room " << currentRoom_->getRoomId() << " (" << currentRoom_->getRoomName() << ")." << std::endl;
                    }
                }
            } else {
                std::cerr << "Robot " << name << ": Error - nextRoom_ is null when movementProgress_ <= 0." << std::endl;
            }
        }
    }

    else if (!movementQueue_.empty()) {
        // Start moving to the next room
        nextRoom_ = movementQueue_.front();
        movementQueue_.pop();
        movementProgress_ = 5.0; // Movement time in seconds
        std::cout << "Robot " << name << " started moving to room " << nextRoom_->getRoomId() << " (" << nextRoom_->getRoomName() << ")." << std::endl;
    }

    else if (isCleaning()) {
        // Continue cleaning
        depleteBattery(1); // Deplete battery for cleaning
        depleteWater(1); // Deplete water for cleaning
        cleaningTimeRemaining_ -= 1.0; // Assuming update is called every second
        std::cout << "Robot " << name << " is cleaning. Battery level: " << batteryLevel << "%" << std::endl;
        if (cleaningTimeRemaining_ <= 0.0) {
            // Cleaning completed
            stopCleaning();
            std::cout << "Robot " << name << " completed cleaning room " << currentRoom_->getRoomId() << " (" << currentRoom_->getRoomName() << ")." << std::endl;
            // Mark the room as clean
            if (currentRoom_) {
                currentRoom_->markClean();
                std::cout << "Room " << currentRoom_->getRoomName() << " marked as clean." << std::endl;
            }
            // Set path back to charging station
            Room* chargingStation = map.getRoomById(0);
            if (chargingStation && currentRoom_ != chargingStation) {
                std::vector<int> pathToCharger = map.getRoute(*currentRoom_, *chargingStation);
                if (!pathToCharger.empty()) {
                    setMovementPath(pathToCharger, map);
                    setTargetRoom(chargingStation);
                    std::cout << "Robot " << name << " is returning to charging station." << std::endl;
                } else {
                    std::cerr << "Robot " << name << ": Cannot find path back to charging station." << std::endl;
                }
            }
        }
    }

    else if (batteryLevel <= 20 && !isCharging_) {
        // Battery low, return to charger
        Room* chargingStation = map.getRoomById(0);
        if (chargingStation && currentRoom_ != chargingStation) {
            std::vector<int> pathToCharger = map.getRoute(*currentRoom_, *chargingStation);
            if (!pathToCharger.empty()) {
                setMovementPath(pathToCharger, map);
                setTargetRoom(chargingStation);
                std::cout << "Robot " << name << " battery low. Returning to charging station." << std::endl;
            } else {
                std::cerr << "Robot " << name << ": Cannot find path to charging station." << std::endl;
            }
        } else if (currentRoom_ == chargingStation) {
            startCharging();
            std::cout << "Robot " << name << " started charging at the charging station." << std::endl;
        }
    }
}

void Robot::startCharging() {
    isCharging_ = true;
    chargingTimeRemaining_ = 20; // Charging takes 20 seconds
}

void Robot::stopCharging() {
    isCharging_ = false;
    chargingTimeRemaining_ = 0;
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
    } else if (batteryLevel < 20) {
        status = "Low Battery";
    } else if (waterLevel_ < 20) {
        status = "Low Water";
    }
    return status;
}
