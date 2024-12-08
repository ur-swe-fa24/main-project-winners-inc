#include "RobotSimulator/RobotSimulator.hpp"
#include "Robot/Robot.h"
#include "Scheduler/Scheduler.hpp"
#include "AlertSystem/alert_system.h"
#include "map/map.h"
#include "CleaningTask/cleaningTask.h"
#include "alert/Alert.h"
#include "adapter/MongoDBAdapter.hpp" // Ensure included if needed
#include <iostream>
#include <algorithm>
#include <ctime>

RobotSimulator::RobotSimulator(std::shared_ptr<Map> map,
                               std::shared_ptr<Scheduler> scheduler,
                               std::shared_ptr<AlertSystem> alertSystem,
                               std::shared_ptr<MongoDBAdapter> dbAdapter)
    : map_(map), scheduler_(scheduler), alertSystem_(alertSystem), dbAdapter_(dbAdapter) {}

std::shared_ptr<Robot> RobotSimulator::getRobotByName(const std::string& name) {
    for (auto& r : robots_) {
        if (r->getName() == name) return r;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Robot>>& RobotSimulator::getRobots() {
    return robots_;
}

void RobotSimulator::update(double deltaTime) {
    for (auto& robot : robots_) {
        bool wasCleaning = robot->isCleaning();
        robot->updateState(deltaTime);
        bool nowCleaning = robot->isCleaning();

        // If robot just finished cleaning a task
        if (wasCleaning && !nowCleaning && !robot->getCurrentTask()) {
            // Robot completed a task, attempt to get next task
            if (scheduler_) {
                const auto& allTasks = scheduler_->getAllTasks();
                std::cout << "Debug: Currently, there are " << allTasks.size()
                          << " task(s) in the scheduler.\n";
                std::cout << "Debug: Trying to get next task for robot "
                          << robot->getName() << "...\n";

                auto nextTask = scheduler_->getNextTaskForRobot(robot->getName());
                std::cout << "Debug: nextTask is " << (nextTask ? "not null" : "null") << "\n";

                if (nextTask) {
                    robot->setCurrentTask(nextTask);
                    assignTaskToRobot(nextTask);
                } else {
                    handleNoTaskAndReturnToChargerIfNeeded(robot);
                }
            } else {
                handleNoTaskAndReturnToChargerIfNeeded(robot);
            }
        }
    }

    checkRobotStatesAndSendAlerts();
}

void RobotSimulator::handleNoTaskAndReturnToChargerIfNeeded(std::shared_ptr<Robot> robot) {
    double battery = robot->getBatteryLevel();
    double water = robot->getWaterLevel();

    bool noTasksLeft = true; 
    bool lowResources = (battery < 20.0 || water <= 0);
    bool needsReturn = (noTasksLeft || lowResources);

    if (needsReturn) {
        std::cout << "Debug: Robot " << robot->getName()
                  << " has no tasks and/or low resources, returning to charger.\n";
        requestReturnToCharger(robot->getName());
    } else {
        std::cout << "Debug: Robot " << robot->getName()
                  << " has no tasks but does not need charger right now.\n";
    }
}

void RobotSimulator::moveRobotToRoom(const std::string& robotName, int roomId) {
    auto robot = getRobotByName(robotName);
    if (!robot) return;
    Room* currentRoom = robot->getCurrentRoom();
    Room* targetRoom = map_->getRoomById(roomId);
    if (!currentRoom || !targetRoom) return;

    auto route = map_->getRoute(*currentRoom, *targetRoom);
    if (route.empty()) {
        if (alertSystem_) {
            alertSystem_->sendAlert("No path found for robot " + robotName, "Movement");
            // Optionally save alert
            if (dbAdapter_) {
                // Convert currentRoom to shared_ptr<Room>
                std::shared_ptr<Room> curRoomPtr = currentRoom ? std::make_shared<Room>(*currentRoom) : nullptr;
                Alert alert("Movement", "No path found for robot " + robotName, 
                            getRobotByName(robotName), curRoomPtr, std::time(nullptr), Alert::LOW);
                dbAdapter_->saveAlert(alert);
            }
        }
        return;
    }

    robot->setMovementPath(route, *map_);
}

void RobotSimulator::startRobotCleaning(const std::string& robotName) {
    auto robot = getRobotByName(robotName);
    if (!robot) return;
    robot->startCleaning(CleaningTask::VACUUM); 
}

void RobotSimulator::stopRobotCleaning(const std::string& robotName) {
    auto robot = getRobotByName(robotName);
    if (!robot) return;
    robot->stopCleaning();
}

void RobotSimulator::manuallyPickUpRobot(const std::string& robotName) {
    auto robot = getRobotByName(robotName);
    if (!robot) return;
    Room* charger = map_->getRoomById(0);
    if (!charger) return;
    robot->setCurrentRoom(charger);
    robot->setCharging(true);
}

void RobotSimulator::requestReturnToCharger(const std::string& robotName) {
    auto robot = getRobotByName(robotName);
    if (!robot) return;
    Room* charger = map_->getRoomById(0);
    if (!charger) return;

    auto route = map_->getRoute(*robot->getCurrentRoom(), *charger);
    if (!route.empty()) {
        robot->setMovementPath(route, *map_);
    } else {
        robot->setCurrentRoom(charger);
        robot->setCharging(true);
    }
}

std::vector<RobotSimulator::RobotStatus> RobotSimulator::getRobotStatuses() const {
    std::vector<RobotStatus> statuses;
    for (auto& robot : robots_) {
        RobotStatus rs {
            robot->getName(),
            robot->getBatteryLevel(),
            robot->getWaterLevel(),
            robot->getCurrentRoom() ? robot->getCurrentRoom()->getRoomName() : "Unknown",
            robot->isCleaning(),
            robot->needsCharging(),
            robot->getStatus()
        };
        statuses.push_back(rs);
    }
    return statuses;
}

const Map& RobotSimulator::getMap() const {
    return *map_;
}

std::shared_ptr<AlertSystem> RobotSimulator::getAlertSystem() const {
    return alertSystem_;
}

void RobotSimulator::checkRobotStatesAndSendAlerts() {
    for (auto& robot : robots_) {
        // Battery alert
        if (robot->needsCharging() && !robot->isLowBatteryAlertSent()) {
            if (alertSystem_) {
                alertSystem_->sendAlert("Robot " + robot->getName() + " has low battery.", "Battery");
            }
            if (dbAdapter_) {
                std::shared_ptr<Room> curRoomPtr = robot->getCurrentRoom() ? std::make_shared<Room>(*robot->getCurrentRoom()) : nullptr;
                Alert alert("Battery", "Robot " + robot->getName() + " has low battery.", robot, curRoomPtr, std::time(nullptr), Alert::HIGH);
                dbAdapter_->saveAlert(alert);
            }
            robot->setLowBatteryAlertSent(true);
        }

        // Water alert
        if (robot->needsWaterRefill() && !robot->isLowWaterAlertSent()) {
            if (alertSystem_) {
                alertSystem_->sendAlert("Robot " + robot->getName() + " has low water.", "Water");
            }
            if (dbAdapter_) {
                std::shared_ptr<Room> curRoomPtr = robot->getCurrentRoom() ? std::make_shared<Room>(*robot->getCurrentRoom()) : nullptr;
                Alert alert("Water", "Robot " + robot->getName() + " has low water.", robot, curRoomPtr, std::time(nullptr), Alert::HIGH);
                dbAdapter_->saveAlert(alert);
            }
            robot->setLowWaterAlertSent(true);
        }
    }
}


void RobotSimulator::addRobot(const std::string& robotName) {
    Room* charger = map_->getRoomById(0);
    // Default to MEDIUM size and VACUUM strategy if none specified
    auto newRobot = std::make_shared<Robot>(robotName, 100.0, Robot::Size::MEDIUM, Robot::Strategy::VACUUM, 100.0);
    if (charger) newRobot->setCurrentRoom(charger);
    newRobot->setMap(map_.get()); 
    robots_.push_back(newRobot);
}

void RobotSimulator::assignTaskToRobot(std::shared_ptr<CleaningTask> task) {
    auto robot = task->getRobot();
    if (!robot) return;

    Room* currentRoom = robot->getCurrentRoom();
    Room* targetRoom = task->getRoom();
    if (!currentRoom || !targetRoom) return;

    robot->setTargetRoom(targetRoom);
    auto route = map_->getRoute(*currentRoom, *targetRoom);
    if (!route.empty()) {
        robot->setMovementPath(route, *map_);
    }
}

// In Scheduler, make sure when assigning tasks, we save tasks alerts as before
// using the code snippet provided by the user.

