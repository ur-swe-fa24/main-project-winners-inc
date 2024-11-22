#include "Schedular/Schedular.hpp"
#include <iostream>
#include <stdexcept>

Scheduler::Scheduler(Map* map, std::vector<std::shared_ptr<Robot>>* robots)
    : map_(map), robots_(robots) {}

void Scheduler::assignCleaningTask(const std::string& robotName, int targetRoomId, const std::string& cleaningStrategy) {
    if (!map_ || !robots_) {
        throw std::runtime_error("Scheduler not properly initialized");
    }

    auto robot = findRobotByName(robotName);
    if (!robot) {
        throw std::runtime_error("Robot " + robotName + " not found.");
    }

    Room* targetRoom = map_->getRoomById(targetRoomId);
    if (!targetRoom) {
        throw std::runtime_error("Target room " + std::to_string(targetRoomId) + " not found.");
    }

    // Get route to target room
    std::vector<int> route = map_->getRoute(*robot->getCurrentRoom(), *targetRoom);
    if (route.empty()) {
        throw std::runtime_error("No available route for robot " + robotName + " to room " + std::to_string(targetRoomId) + ".");
    }

    // Set the movement path for the robot
    robot->setMovementPath(route, *map_);

    // Set the target room
    robot->setTargetRoom(targetRoom);

    std::cout << "Assigned task to robot " << robotName << " to clean room " << targetRoom->getRoomId() << " using strategy " << cleaningStrategy << "." << std::endl;
}

void Scheduler::executeCleaning(std::shared_ptr<Robot> robot, Room* targetRoom, const std::string& strategy) {
    if (!map_ || !robots_) {
        throw std::runtime_error("Scheduler not properly initialized");
    }

    // Get route to target room
    std::vector<int> route = map_->getRoute(*robot->getCurrentRoom(), *targetRoom);

    for (int roomId : route) {
        Room* nextRoom = map_->getRoomById(roomId);
        if (robot->moveToRoom(nextRoom)) {
            // Battery depletes from movement
            robot->depleteBattery(5); // Example energy cost per room transition
        }
    }

    // Simulate cleaning task based on room type
    int cleaningTime = getCleaningTime(*targetRoom);
    robot->depleteBattery(cleaningTime); // Battery depletes based on cleaning time
    targetRoom->markClean();
    std::cout << "Robot " << robot->getName() << " cleaned room " << targetRoom->getRoomName() << " with strategy " << strategy << std::endl;
}

int Scheduler::getCleaningTime(const Room& room) const {
    if (room.flooringType == "Hardwood") return 20; // Example cleaning time
    if (room.flooringType == "Carpet") return 30;
    return 15; // Default cleaning time
}

std::shared_ptr<Robot> Scheduler::findRobotByName(const std::string& name) {
    if (!robots_) {
        return nullptr;
    }
    
    for (const auto& robot : *robots_) {
        if (robot->getName() == name) {
            return robot;
        }
    }
    return nullptr;
}
