#include "Scheduler/Scheduler.hpp"
#include "map/map.h"
#include "Robot/Robot.h"
#include "RobotSimulator/RobotSimulator.hpp"
#include "AlertSystem/alert_system.h"
#include "adapter/MongoDBAdapter.hpp"
#include "alert/Alert.h"
#include "AlertDialog/AlertDialog.hpp"
#include <algorithm>
#include <stdexcept>
#include <ctime>
#include <iostream>

void Scheduler::addTask(std::shared_ptr<CleaningTask> task) {
    tasks_.push_back(task);
    std::cout << "[DEBUG] Scheduler::addTask: Added task " << task->getID() << "\n";
    printTasks();
}

std::shared_ptr<CleaningTask> Scheduler::getNextTaskForRobot(const std::string& robotName) {
    auto robot = findRobotByName(robotName);
    if (!robot) return nullptr;

    std::cout << "[DEBUG] Scheduler::getNextTaskForRobot for robot " << robotName << "\n";
    printTasks();

    // Find the first Pending task for this robot
    auto it = std::find_if(tasks_.begin(), tasks_.end(), 
        [&robot](const std::shared_ptr<CleaningTask>& t) {
            return t->getRobot() == robot && t->getStatus() == "Pending";
        });

    if (it == tasks_.end()) {
        std::cout << "[DEBUG] No pending tasks for " << robotName << ". Returning robot to charger.\n";
        checkAndReturnToChargerIfNeeded(robot);
        return nullptr;
    }

    auto task = *it;
    tasks_.erase(it);
    std::cout << "[DEBUG] Scheduler::getNextTaskForRobot: Assigning task " << task->getID() << " to " << robotName << "\n";
    printTasks();

    return task;
}

void Scheduler::requeueTask(std::shared_ptr<CleaningTask> task) {
    tasks_.push_back(task);
    std::cout << "[DEBUG] Scheduler::requeueTask: Requeued task " << task->getID() << "\n";
    printTasks();
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
    if (robot->isFailed()) {
        throw std::runtime_error("Cannot assign task to a failed robot.");
    }

    // Check if the robot already has a current task
    if (robot->getCurrentTask()) {
        std::cout << "[DEBUG] Robot " << robotName << " already has a current task. Not assigning a new one." << std::endl;
        return; // or throw an exception, or handle differently
    }

    CleaningTask::CleanType ctype = CleaningTask::stringToCleanType(strategy);
    auto task = std::make_shared<CleaningTask>(++taskIdCounter_, CleaningTask::MEDIUM, ctype, selectedRoom);

    task->assignRobot(robot);
    addTask(task);
    robot->setCurrentTask(task);

    if (simulator_) {
        simulator_->assignTaskToRobot(task);
    }

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
        dbAdapter_->saveAlert(newAlert);
    }

    std::cout << "[DEBUG] Scheduler::assignCleaningTask: Assigned new task " << task->getID() 
              << " to " << robotName << " for room " << selectedRoom->getRoomName() << std::endl;
    printTasks();
}


const std::vector<std::shared_ptr<CleaningTask>>& Scheduler::getAllTasks() const {
    return tasks_;
}

void Scheduler::checkAndReturnToChargerIfNeeded(std::shared_ptr<Robot> robot) {
    double battery = robot->getBatteryLevel();
    double water = robot->getWaterLevel();
    bool noTasksLeft = true;
    for (auto& t : tasks_) {
        if (t->getRobot() == robot && t->getStatus() == "Pending") {
            noTasksLeft = false;
            break;
        }
    }

    bool needsReturn = noTasksLeft || battery < 20.0 || water < 20.0;
    std::string name = robot->getName();
    Room* errorRoom = robot->getCurrentRoom();

    if (needsReturn && simulator_) {
        std::cout << "[DEBUG] " << robot->getName() << " returning to charger.\n";
        simulator_->requestReturnToCharger(robot->getName());

        if (alertSystem_ && (battery < 20.0)){
            std::string message = name + " has low battery levels. Returning to charger.";
            alertSystem_->sendAlert(message, "Task");
        }

        if (alertSystem_ && (water < 20.0)) {
            std::string message = name + " has low water levels. Refill tank.";
            alertSystem_->sendAlert(message, "Task");
        }

        if (dbAdapter_ && (battery < 20.0)) {
            Alert newAlert("Task",
                           name + " has low battery levels. Returning to charger.",
                           robot,
                           std::make_shared<Room>(*errorRoom),
                           std::time(nullptr),
                           Alert::LOW);
            dbAdapter_->saveAlert(newAlert);
        }

        if (dbAdapter_ && (water < 20.0)) {
            Alert newAlert("Task",
                           name + " has low water levels. Refill tank.",
                           robot,
                           std::make_shared<Room>(*errorRoom),
                           std::time(nullptr),
                           Alert::LOW);
            dbAdapter_->saveAlert(newAlert);
        }
    }
}

void Scheduler::printTasks() const {
    std::cout << "[DEBUG] Current Task List:\n";
    if (tasks_.empty()) {
        std::cout << "  No tasks.\n";
        return;
    }
    for (auto& t : tasks_) {
        std::cout << "  Task ID: " << t->getID() 
                  << ", Status: " << t->getStatus()
                  << ", Robot: " << (t->getRobot() ? t->getRobot()->getName() : "None")
                  << ", Room: " << (t->getRoom() ? t->getRoom()->getRoomName() : "Unknown")
                  << "\n";
    }
}
