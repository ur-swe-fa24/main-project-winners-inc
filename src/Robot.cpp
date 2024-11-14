#include "Robot/Robot.h"
#include "map/map.h"
#include <iostream>

// Constructor implementation
Robot::Robot(const std::string& name, int batteryLevel)
    : name(name), batteryLevel(batteryLevel), cleaning_(false), lowBatteryAlertSent_(false), currentRoom_(nullptr), 
    nextRoom_(nullptr), targetRoom_(nullptr), isCharging_(false),
      chargingTimeRemaining_(0), movementProgress_(0.0), cleaningTimeRemaining_(0.0)  {}

// Methods to start and stop cleaning
void Robot::startCleaning() {
    cleaning_ = true;
    // Adjust cleaning time based on room type
    if (currentRoom_->flooringType == "Hardwood") {
        cleaningTimeRemaining_ = 15.0;
    } else if (currentRoom_->flooringType == "Carpet") {
        cleaningTimeRemaining_ = 20.0;
    } else {
        cleaningTimeRemaining_ = 10.0; // Default cleaning time
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

// void Robot::update() {
//     // if (movementProgress_ > 0) {
//     //     movementProgress_ -= 0.5; // Since timer is every 0.5 seconds
//     //     if (movementProgress_ <= 0) {
//     //         movementProgress_ = 0;
//     //         // Movement to next room complete
//     //         currentRoom_ = nextRoom_;
//     //         nextRoom_ = nullptr;
//     //         depleteBattery(1);
//     //     }
//     // }

//     // if (isCharging_) {
//     //     if (chargingTimeRemaining_ > 0) {
//     //         chargingTimeRemaining_--;
//     //         batteryLevel += 5; // Increase battery level per second
//     //         if (batteryLevel > 100) batteryLevel = 100;
//     //     } else {
//     //         // Charging complete
//     //         stopCharging();
//     //     }
//     if (isCharging_) {
//         // Charging logic
//         batteryLevel += 5; // Increase battery level per update
//         if (batteryLevel >= 100) {
//             batteryLevel = 100;
//             stopCharging();
//         }

//     } else if (movementProgress_ > 0.0) {
//         // Robot is moving between rooms
//         movementProgress_ -= 0.5; // Decrement by 0.5 for 500ms intervals
//         if (movementProgress_ <= 0.0) {
//             // Movement to next room complete
//             movementProgress_ = 0.0;
//             currentRoom_ = nextRoom_;
//             nextRoom_ = nullptr;
//             depleteBattery(1);

//             // Check if we've arrived at the target room
//             if (currentRoom_ == targetRoom_) {
//                 startCleaning();
//             }
//         }
//     } else if (!movementQueue_.empty()) {
//         // Start moving to the next room
//         nextRoom_ = movementQueue_.front();
//         movementQueue_.pop();
//         movementProgress_ = 10.0; // Movement time in seconds

        
//     } else if (isCleaning()) {
//         depleteBattery(1); // Deplete battery for cleaning
//     }
// }

void Robot::update(const Map& map) {
    if (isCharging_) {
        // Charging logic
        batteryLevel += 5; // Increase battery level per update
        if (batteryLevel >= 100) {
            batteryLevel = 100;
            stopCharging();
        }
    } else if (movementProgress_ > 0.0) {
        // Movement logic
        movementProgress_ -= 0.5;
        if (movementProgress_ <= 0.0) {
            movementProgress_ = 0.0;
            currentRoom_ = nextRoom_;
            nextRoom_ = nullptr;
            depleteBattery(5);
            if (currentRoom_ == targetRoom_) {
                startCleaning();
            }
        }
    } else if (!movementQueue_.empty()) {
        // Start moving to the next room
        nextRoom_ = movementQueue_.front();
        movementQueue_.pop();
        movementProgress_ = 10.0; // Movement time in seconds
    } else if (isCleaning()) {
        // Continue cleaning
        depleteBattery(1); // Deplete battery for cleaning
        cleaningTimeRemaining_ -= 0.5;
        if (cleaningTimeRemaining_ <= 0.0) {
            // Cleaning completed
            stopCleaning();
            currentRoom_->markClean();
            // Set path back to charging station
            Room* chargingStation = map.getRoomById(0);
            if (chargingStation && currentRoom_ != chargingStation) {
                std::vector<int> pathToCharger = map.getRoute(*currentRoom_, *chargingStation);
                setMovementPath(pathToCharger, *map);
                setTargetRoom(chargingStation);
            }
        }
    } else if (batteryLevel <= 20 && !isCharging_) {
        // Battery low, return to charger
        Room* chargingStation = map.getRoomById(0);
        if (chargingStation && currentRoom_ != chargingStation) {
            std::vector<int> pathToCharger = map.getRoute(*currentRoom_, *chargingStation);
            setMovementPath(pathToCharger, *map);
            setTargetRoom(chargingStation);
        } else if (currentRoom_ == chargingStation) {
            startCharging();
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
    if (isCharging_) {
        return "Charging";
    } else if (movementProgress_ > 0 || !movementQueue_.empty()) {
        if (currentRoom_ && currentRoom_->getRoomId() == 0) {
            return "Returning to Charger";
        }
        return "Moving";
    } else if (cleaning_) {
        return "Cleaning";
    } else {
        return "Idle";
    }
}
