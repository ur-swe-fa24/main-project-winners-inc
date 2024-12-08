#include "RobotSimulator/RobotSimulator.hpp"
#include "Robot/Robot.h"
#include "Scheduler/Scheduler.hpp"
#include "AlertSystem/alert_system.h"
#include "map/map.h"
#include "CleaningTask/cleaningTask.h"
#include <iostream>
#include <algorithm>

RobotSimulator::RobotSimulator(std::shared_ptr<Map> map,
                               std::shared_ptr<Scheduler> scheduler,
                               std::shared_ptr<AlertSystem> alertSystem)
    : map_(map), scheduler_(scheduler), alertSystem_(alertSystem) {}

std::shared_ptr<Robot> RobotSimulator::getRobotByName(const std::string& name) {
    for (auto& r : robots_) {
        if (r->getName() == name) return r;
    }
    return nullptr;
}

void RobotSimulator::update(double deltaTime) {
    // Update each robot
    for (auto& robot : robots_) {
        bool wasCleaning = robot->isCleaning();
        robot->updateState(deltaTime);
        bool nowCleaning = robot->isCleaning();

        // Check if robot just finished cleaning a task
        if (wasCleaning && !nowCleaning && !robot->getCurrentTask()) {
            // Robot completed a task, attempt to get next task
            if (scheduler_) {
                auto nextTask = scheduler_->getNextTaskForRobot(robot->getName());
                if (nextTask) {
                    // Got a new task, assign and set path
                    robot->setCurrentTask(nextTask);
                    assignTaskToRobot(nextTask);
                } else {
                    // No new tasks returned for this robot, handle return to charger if needed
                    handleNoTaskAndReturnToChargerIfNeeded(robot);
                }
            } else {
                // No scheduler: just handle return to charger logic
                handleNoTaskAndReturnToChargerIfNeeded(robot);
            }
        }
    }

    checkRobotStatesAndSendAlerts();
}

void RobotSimulator::handleNoTaskAndReturnToChargerIfNeeded(std::shared_ptr<Robot> robot) {
    // If no tasks left or robot resources are low, return to charger
    double battery = robot->getBatteryLevel();
    double water = robot->getWaterLevel();

    // Conditions: If no tasks are available for this robot, we consider returning to charger.
    // Even if battery/water is okay, you might decide to return it anyway. 
    // Let's return to charger only if low resources to be consistent with previous logic:
    bool needsReturn = (battery < 20.0 || water < 0);

    // If you always want to return to charger when no tasks are left, do: bool needsReturn = true;
    // Adjust as needed. For now, let's stick to resource logic:
    if (needsReturn) {
        requestReturnToCharger(robot->getName());
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
        // If no route, move robot directly to charger and start charging
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
        if (robot->needsCharging()) {
            if (alertSystem_) {
                alertSystem_->sendAlert("Robot " + robot->getName() + " has low battery.", "Battery");
            }
        }
        if (robot->needsWaterRefill()) {
            if (alertSystem_) {
                alertSystem_->sendAlert("Robot " + robot->getName() + " has low water.", "Water");
            }
        }
    }
}

void RobotSimulator::addRobot(const std::string& robotName) {
    Room* charger = map_->getRoomById(0);
    auto newRobot = std::make_shared<Robot>(robotName, 100.0, 100.0);
    if (charger) newRobot->setCurrentRoom(charger);
    robots_.push_back(newRobot);
}

const std::vector<std::shared_ptr<Robot>>& RobotSimulator::getRobots() const {
    return robots_;
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
