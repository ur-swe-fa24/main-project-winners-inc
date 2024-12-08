#include "Scheduler/Scheduler.hpp"
#include "map/map.h"
#include "Robot/Robot.h"
#include "RobotSimulator/RobotSimulator.hpp"
#include "AlertSystem/alert_system.h"
#include "adapter/MongoDBAdapter.hpp"
#include "alert/Alert.h"  // Make sure this is included for the Alert class
#include <algorithm>
#include <stdexcept>
#include <ctime>

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
    Room* selectedRoom = map_->getRoomById(targetRoomId);
    if (!selectedRoom) throw std::runtime_error("Target room not found.");

    auto robot = findRobotByName(robotName);
    if (!robot) throw std::runtime_error("Robot not found.");

    CleaningTask::CleanType ctype = CleaningTask::stringToCleanType(strategy);
    auto task = std::make_shared<CleaningTask>(++taskIdCounter_, CleaningTask::MEDIUM, ctype, selectedRoom);

    task->assignRobot(robot);
    addTask(task);
    robot->setCurrentTask(task);

    if (simulator_) {
        simulator_->assignTaskToRobot(task);
    }

    // Send alert and save to DB
    if (alertSystem_) {
        alertSystem_->sendAlert("Task assigned to robot " + robotName + " for room " + selectedRoom->getRoomName(), "Task");
    }
    if (dbAdapter_) {
        Alert newAlert("Task", 
                       "Task assigned to robot " + robotName + " for room " + selectedRoom->getRoomName(),
                       robot,
                       std::make_shared<Room>(*selectedRoom),
                       std::time(nullptr),
                       Alert::LOW);
        dbAdapter_->saveAlertAsync(newAlert);
    }
}

const std::vector<std::shared_ptr<CleaningTask>>& Scheduler::getAllTasks() const {
    return tasks_;
}

void Scheduler::checkAndReturnToChargerIfNeeded(std::shared_ptr<Robot> robot) {
    if (!simulator_) return;

    double battery = robot->getBatteryLevel();
    double water = robot->getWaterLevel();

    // Return to charger if no tasks left OR battery < 20% or water < 20%
    bool noTasksLeft = tasks_.empty();
    bool needsReturn = noTasksLeft || battery < 20.0 || water < 20.0;

    if (needsReturn) {
        simulator_->requestReturnToCharger(robot->getName());
    }
}
