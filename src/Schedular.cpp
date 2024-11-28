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

    // Create a new cleaning task
    static int taskIdCounter = 0;
    auto task = std::make_shared<CleaningTask>(++taskIdCounter, CleaningTask::MEDIUM, 
        cleaningStrategyFromString(cleaningStrategy), targetRoom);

    // Assign the robot to the task
    task->assignRobot(robot);

    // Add the task to the robot's queue
    robot->addTaskToQueue(task);

    // Add the task to the global task list
    tasks_.push_back(task);


    std::cout << "Assigned task to robot " << robotName << " to clean room " << targetRoom->getRoomId() 
              << " using strategy " << cleaningStrategy << "." << std::endl;
}

void Scheduler::update() {
    if (!map_ || !robots_) {
        return;
    }

    // Update each robot
    for (auto& robot : *robots_) {
        if (robot) {
            robot->update(*map_);
        }
    }

    // Remove completed tasks from the task list
    tasks_.erase(
        std::remove_if(tasks_.begin(), tasks_.end(),
            [](const std::shared_ptr<CleaningTask>& task) {
                return task->getStatus() == "Completed";
            }),
        tasks_.end()
    );
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
    std::cout << "Robot " << robot->getName() << " cleaned room " << targetRoom->getRoomName() 
              << " with strategy " << strategy << std::endl;
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

// Helper function to convert string to CleanType enum
CleaningTask::CleanType Scheduler::cleaningStrategyFromString(const std::string& strategy) {
    std::string lowerStrategy = strategy;
    std::transform(lowerStrategy.begin(), lowerStrategy.end(), lowerStrategy.begin(), ::tolower);
    if (lowerStrategy == "vacuum") return CleaningTask::VACUUM;
    if (lowerStrategy == "scrub") return CleaningTask::SCRUB;
    if (lowerStrategy == "shampoo") return CleaningTask::SHAMPOO;
    throw std::runtime_error("Unknown cleaning strategy: " + strategy);
}

const std::vector<std::shared_ptr<CleaningTask>>& Scheduler::getAllTasks() const {
    return tasks_;
}

