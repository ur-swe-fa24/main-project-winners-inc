#include "Robot/Robot.h"
#include "CleaningTask/cleaningTask.h"
#include "Room/Room.h"
#include "map/map.h"
#include <algorithm>
#include <iostream>

Robot::Robot(const std::string& name, double batteryLevel, Size size, Strategy strategy, double waterLevel)
    : name_(name), batteryLevel_(batteryLevel), waterLevel_(waterLevel),
      cleaning_(false), isCharging_(false), cleaningProgress_(0.0), movementProgress_(0.0),
      currentRoom_(nullptr), nextRoom_(nullptr), cleaningTimeRemaining_(0.0),
      targetRoom_(nullptr), lowBatteryAlertSent_(false), lowWaterAlertSent_(false),
      currentTask_(nullptr), savedTask_(nullptr), savedCleaningTimeRemaining_(0.0),
      size_(size), strategy_(strategy), robotMap_(nullptr),
      errorCount_(0), totalWorkTime_(0.0), failed_(false) {}

void Robot::updateState(double deltaTime) {
    std::cout << "[DEBUG] Robot " << name_ << " updateState: Battery=" << batteryLevel_
              << "%, Water=" << waterLevel_ << "%, CurrentTask=" 
              << (currentTask_ ? std::to_string(currentTask_->getID()) : "None")
              << ", Status=" << getStatus() << "\n";

    if (failed_) {
        std::cout << "[DEBUG] Robot " << name_ << " is failed, no operation.\n";
        return;
    }

    if (cleaning_) {
        double failChance = 0.01;
        double rnd = (double)rand() / RAND_MAX;
        if (rnd < failChance && !failed_) {
            failed_ = true;
            errorCount_++;
            std::cout << "[DEBUG] Robot " << name_ << " failed during cleaning!\n";
            return;
        }
    }

    if (isCleaning() || isMoving()) {
        totalWorkTime_ += deltaTime;
    }

    if (batteryLevel_ <= 0.0) {
        if (cleaning_) stopCleaning();
        nextRoom_ = nullptr;
        return; 
    }

    if (isCharging_) {
        batteryLevel_ = std::min(100.0, batteryLevel_ + deltaTime * 20.0);
        if (batteryLevel_ >= 100.0) {
            isCharging_ = false;
            if (waterLevel_ < 100.0) {
                refillWater();
            }
            fullyRecharge();
        }
    } else {
        if (cleaning_ && currentTask_) {
            double batteryDepletion = 5.0 * deltaTime;
            batteryLevel_ = std::max(0.0, batteryLevel_ - batteryDepletion);
            if (currentTask_->getCleanType() == CleaningTask::SHAMPOO) {
                double waterDepletion = 5.0 * deltaTime;
                waterLevel_ = std::max(0.0, waterLevel_ - waterDepletion);
            }

            if ((needsCharging() || needsWaterRefill()) && cleaning_) {
                std::cout << "[DEBUG] Robot " << name_ << " resources low mid-cleaning, saving task.\n";
                saveCurrentTask();
                stopCleaning();
            }
        }
    }

    if (isMoving()) {
        movementProgress_ += deltaTime * 10.0;
        if (movementProgress_ >= 100.0) {
            currentRoom_ = nextRoom_;
            nextRoom_ = nullptr;
            movementProgress_ = 0.0;

            if (!movementQueue_.empty()) {
                nextRoom_ = movementQueue_.front();
                movementQueue_.pop();
            } else {
                if (currentTask_ && targetRoom_ == currentRoom_ && !cleaning_) {
                    startCleaning(currentTask_->getCleanType());
                }
            }
        }
    }

    if (cleaning_ && currentTask_) {
        cleaningTimeRemaining_ -= deltaTime;
        if (cleaningTimeRemaining_ <= 0) {
            std::cout << "[DEBUG] Robot " << name_ << " finished cleaning task " << currentTask_->getID() << ".\n";
            cleaning_ = false;
            currentTask_->markCompleted();
            currentTask_.reset();
            cleaningProgress_ = 0.0;
            if (currentRoom_) {
                currentRoom_->markClean();
                std::cout << "[DEBUG] Room " << currentRoom_->getRoomName() << " is now clean.\n";
            }
        }
    }

    // if (needsCharging() && !lowBatteryAlertSent_) {
    //     lowBatteryAlertSent_ = true;
    // }
    // if (needsWaterRefill() && !lowWaterAlertSent_) {
    //     lowWaterAlertSent_ = true;
    // }

    if (currentRoom_ && currentRoom_->getRoomId() == 0 && batteryLevel_ < 100.0 && !isCharging_) {
        std::cout << "[DEBUG] Robot " << name_ << " at charger, starting charge.\n";
        setCharging(true);
    }
}

void Robot::startCleaning(CleaningTask::CleanType cleaningType) {
    std::cout << "[DEBUG] Robot " << name_ << " attempting to start cleaning.\n";
    if (isCleaning() || !currentRoom_ || !currentTask_) {
        std::cout << "[DEBUG] Robot " << name_ << " cannot start cleaning now.\n";
        return;
    }
    if (batteryLevel_ < 20.0 || waterLevel_ <= 0.0) {
        std::cout << "[DEBUG] Robot " << name_ << " not enough resources to start cleaning.\n";
        saveCurrentTask();
        return;
    }
    cleaning_ = true;
    if (currentTask_) {
        currentTask_->setStatus("In Progress");
    }

    double baseTime = 15.0;
    std::string size = currentRoom_->getSize();
    std::transform(size.begin(), size.end(), size.begin(), ::tolower);

    if (size == "small") baseTime = 5.0;
    else if (size == "medium") baseTime = 10.0;
    else if (size == "large") baseTime = 15.0;

    cleaningTimeRemaining_ = baseTime;
    if (savedTask_ && savedTask_ == currentTask_) {
        cleaningTimeRemaining_ = savedCleaningTimeRemaining_;
        savedTask_.reset();
        savedCleaningTimeRemaining_ = 0.0;
    }

    std::cout << "[DEBUG] Robot " << name_ << " started cleaning task " 
              << currentTask_->getID() << " now In Progress.\n";
}

void Robot::stopCleaning() {
    if (cleaning_) {
        cleaning_ = false;
        currentTask_.reset();
        cleaningProgress_ = 0.0;
    }
}

void Robot::setMovementPath(const std::vector<int>& roomIds, const Map& map) {
    while (!movementQueue_.empty()) movementQueue_.pop();

    for (int roomId : roomIds) {
        Room* r = map.getRoomById(roomId);
        if (r) {
            movementQueue_.push(r);
        }
    }

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
    if (batteryLevel_ <= 0.0) return; // Can't move if no battery
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
void Robot::setCharging(bool charging) { 
    isCharging_ = charging; 
    if (!charging && batteryLevel_ >= 100.0 && waterLevel_ >= 100.0) {
        resumeSavedTask();
    }
}
void Robot::refillWater() { 
    waterLevel_ = 100.0; 
    lowWaterAlertSent_ = false; 
    std::cout << "Robot " << name_ << " water refilled at charger.\n";
}
void Robot::fullyRecharge() { 
    batteryLevel_ = 100.0;
    lowBatteryAlertSent_ = false;
    resumeSavedTask();
}

bool Robot::isCharging() const { return isCharging_; }
double Robot::getMovementProgress() const { return movementProgress_; }
bool Robot::needsMaintenance() const { return false; }
bool Robot::isLowBatteryAlertSent() const { return lowBatteryAlertSent_; }
bool Robot::isLowWaterAlertSent() const { return lowWaterAlertSent_; }


std::string Robot::getStatus() const {
    if (failed_) return "Error";
    if (batteryLevel_ <= 0.0) return "Disabled (No Battery)";
    if (isCharging_) return "Charging";
    if (cleaning_) return "Cleaning";
    if (isMoving()) return "Moving";
    return "Idle";
}

void Robot::setLowBatteryAlertSent(bool val) { lowBatteryAlertSent_ = val; }
void Robot::setLowWaterAlertSent(bool val) { lowWaterAlertSent_ = val; }

void Robot::setCurrentTask(std::shared_ptr<CleaningTask> task) {
    currentTask_ = task;
    if (task) {
        targetRoom_ = task->getRoom();
    }
}

std::shared_ptr<CleaningTask> Robot::getCurrentTask() const {
    return currentTask_;
}

void Robot::saveCurrentTask() {
    if (cleaning_ && currentTask_) {
        savedTask_ = currentTask_;
        savedCleaningTimeRemaining_ = cleaningTimeRemaining_;
        std::cout << "Robot " << name_ << " saved current partial task.\n";
    }
}

bool Robot::resumeSavedTask() {
    if (savedTask_) {
        Room* savedRoom = savedTask_->getRoom();
        if (savedRoom && savedRoom != currentRoom_) {
            std::cout << "Robot " << name_ << " attempting to return to saved task room.\n";
            if (!robotMap_) {
                std::cout << "Robot " << name_ << ": No map reference available to resume task.\n";
                return false;
            }
            // Compute route
            auto route = robotMap_->getRoute(*currentRoom_, *savedRoom);
            if (route.empty()) {
                std::cout << "Robot " << name_ << ": No path to saved task room.\n";
                return false;
            }
            // Set movement path
            setMovementPath(route, *robotMap_);
            // Restore task state but don't start cleaning until arrival
            currentTask_ = savedTask_;
            cleaning_ = false;
            cleaningTimeRemaining_ = savedCleaningTimeRemaining_;
            savedTask_.reset();
            savedCleaningTimeRemaining_ = 0.0;
            return true;
        } else if (savedRoom == currentRoom_) {
            // Resume cleaning immediately
            currentTask_ = savedTask_;
            cleaning_ = true;
            cleaningTimeRemaining_ = savedCleaningTimeRemaining_;
            savedTask_.reset();
            savedCleaningTimeRemaining_ = 0.0;
            std::cout << "Robot " << name_ << " resumed previously saved task.\n";
            return true;
        }
    }
    return false;
}

void Robot::repair() {
    failed_ = false;
}