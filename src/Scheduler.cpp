#include "Scheduler/Scheduler.hpp"
#include "Room/Room.h"
#include "Robot/Robot.h"
#include "map/map.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

void Scheduler::addTask(std::shared_ptr<CleaningTask> task) {
    tasks_.push_back(task);
}

std::shared_ptr<CleaningTask> Scheduler::getNextTaskForRobot(const std::string& /*robotName*/) {
    if (tasks_.empty()) return nullptr;
    auto task = tasks_.front();
    tasks_.erase(tasks_.begin());
    return task;
}

void Scheduler::requeueTask(std::shared_ptr<CleaningTask> task) {
    tasks_.push_back(task);
}

std::shared_ptr<Robot> Scheduler::findRobotByName(const std::string& name) {
    for (auto& r : *robots_) {
        if (r->getName() == name) return r;
    }
    return nullptr;
}

void Scheduler::assignCleaningTask(const std::string& robotName, int targetRoomId, const std::string& strategy) {
    Room* targetRoom = map_->getRoomById(targetRoomId);
    if (!targetRoom) throw std::runtime_error("Target room not found.");

    std::shared_ptr<Robot> robot = findRobotByName(robotName); 
    if (!robot) throw std::runtime_error("Robot not found.");

    CleaningTask::CleanType ctype = CleaningTask::stringToCleanType(strategy);
    auto task = std::make_shared<CleaningTask>(++taskIdCounter_, CleaningTask::MEDIUM, ctype, targetRoom);

    task->assignRobot(robot);
    addTask(task);
    robot->setCurrentTask(task);
    // Robot doesn't move yet, let RobotSimulator set route after assign.

    // If we have access to RobotSimulator here, call simulator->assignTaskToRobot(task).
    // Otherwise, RobotSimulator can monitor tasks and set path itself.
}

const std::vector<std::shared_ptr<CleaningTask>>& Scheduler::getAllTasks() const {
    return tasks_;
}
