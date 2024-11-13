#include "Schedular/Schedular.hpp"
#include <iostream>
#include <stdexcept>

Scheduler::Scheduler(Map& map, std::vector<std::shared_ptr<Robot>>& robots)
    : map_(map), robots_(robots) {}

void Scheduler::assignCleaningTask(const std::string& robotName, int targetRoomId, const std::string& cleaningStrategy) {
    auto robot = findRobotByName(robotName);
    if (!robot) {
        std::cerr << "Robot " << robotName << " not found.\n";
        return;
    }

    Room* targetRoom = map_.getRoomById(targetRoomId);
    if (!targetRoom) {
        std::cerr << "Target room " << targetRoomId << " not found.\n";
        return;
    }

    // Execute cleaning (this could be asynchronous if desired)
    executeCleaning(robot, targetRoom, cleaningStrategy);
}

void Scheduler::executeCleaning(std::shared_ptr<Robot> robot, Room* targetRoom, const std::string& strategy) {
    // Get route to target room
    std::vector<int> route = map_.getRoute(*robot->getCurrentRoom(), *targetRoom);

    for (int roomId : route) {
        Room* nextRoom = map_.getRoomById(roomId);
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
    for (auto& robot : robots_) {
        if (robot->getName() == name) return robot;
    }
    return nullptr;
}
