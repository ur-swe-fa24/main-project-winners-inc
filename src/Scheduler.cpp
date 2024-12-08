// Scheduler.cpp
#include "Scheduler/Scheduler.hpp"
#include "map/map.h"
#include "Robot/Robot.h"
#include "RobotSimulator/RobotSimulator.hpp"
#include <algorithm>
#include <stdexcept>

void Scheduler::addTask(std::shared_ptr<CleaningTask> task) {
    tasks_.push_back(task);
}

std::shared_ptr<CleaningTask> Scheduler::getNextTaskForRobot(const std::string& robotName) {
    auto robot = findRobotByName(robotName);
    if (!robot) return nullptr;

    // Find the first task assigned to this robot
    auto it = std::find_if(tasks_.begin(), tasks_.end(), 
        [&robot](const std::shared_ptr<CleaningTask>& t) {
            return t->getRobot() == robot; // Ensure the task is assigned to this robot
        });

    if (it == tasks_.end()) {
        // No task for this robot
        checkAndReturnToChargerIfNeeded(robot);
        return nullptr;
    }

    // Found a task for this robot
    auto task = *it;
    tasks_.erase(it);
    return task;
}

void Scheduler::requeueTask(std::shared_ptr<CleaningTask> task) {
    tasks_.push_back(task);
}

std::shared_ptr<Robot> Scheduler::findRobotByName(const std::string& name) {
    for (const auto& r : *robots_) {
        if (r->getName() == name) return r;
    }
    return nullptr;
}

void Scheduler::assignCleaningTask(const std::string& robotName, int targetRoomId, const std::string& strategy) {
    Room* targetRoom = map_->getRoomById(targetRoomId);
    if (!targetRoom) throw std::runtime_error("Target room not found.");

    auto robot = findRobotByName(robotName);
    if (!robot) throw std::runtime_error("Robot not found.");

    auto ctype = CleaningTask::stringToCleanType(strategy);
    auto task = std::make_shared<CleaningTask>(++taskIdCounter_, CleaningTask::MEDIUM, ctype, targetRoom);

    task->assignRobot(robot);
    addTask(task);
    robot->setCurrentTask(task);

    if (simulator_) {
        simulator_->assignTaskToRobot(task);
    }
}

const std::vector<std::shared_ptr<CleaningTask>>& Scheduler::getAllTasks() const {
    return tasks_;
}

void Scheduler::checkAndReturnToChargerIfNeeded(std::shared_ptr<Robot> robot) {
    if (!simulator_) return;

    // If no tasks are left for this robot, let's return it to the charger.
    // Additionally, if battery <20% or water <20%, it also should return.
    double battery = robot->getBatteryLevel();
    double water = robot->getWaterLevel();

    // Return to charger if no tasks left OR low battery/water
    bool noTasksLeft = tasks_.empty(); // This method is called only after no tasks for that robot, but double-check
    bool needsReturn = noTasksLeft || battery < 20.0 || water < 20.0;

    if (needsReturn) {
        simulator_->requestReturnToCharger(robot->getName());
    }
}
