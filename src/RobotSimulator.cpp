#include "RobotSimulator/RobotSimulator.hpp"
#include "Robot/Robot.h"
#include "Schedular/Schedular.hpp"
#include "AlertSystem/alert_system.h"
#include "map/map.h"
#include "CleaningTask/cleaningTask.h"
#include <iostream>
#include <algorithm>

RobotSimulator::RobotSimulator(std::shared_ptr<Map> map,
                               std::shared_ptr<Scheduler> scheduler,
                               std::shared_ptr<AlertSystem> alertSystem)
    : map_(map), scheduler_(scheduler), alertSystem_(alertSystem) {}

// Define getRobotByName
std::shared_ptr<Robot> RobotSimulator::getRobotByName(const std::string& name) {
    for (auto& r : robots_) {
        if (r->getName() == name) return r;
    }
    return nullptr;
}

void RobotSimulator::update(double deltaTime) {
    for (auto& robot : robots_) {
        robot->updateState(deltaTime);
        // If robot done cleaning or needs tasks, handle logic here
    }
    checkRobotStatesAndSendAlerts();
}

void RobotSimulator::moveRobotToRoom(const std::string& robotName, int roomId) {
    auto robot = getRobotByName(robotName);
    if (!robot) return;
    Room* currentRoom = robot->getCurrentRoom();
    Room* targetRoom = map_->getRoomById(roomId);
    if (!currentRoom || !targetRoom) return;

    std::vector<int> route = map_->getRoute(*currentRoom, *targetRoom);
    if (route.empty()) {
        alertSystem_->sendAlert("No path found for robot " + robotName, "Movement");
        return;
    }

    robot->setMovementPath(route, *map_);
}

void RobotSimulator::startRobotCleaning(const std::string& robotName) {
    auto robot = getRobotByName(robotName);
    if (!robot) return;
    robot->startCleaning(CleaningTask::VACUUM); // Or choose strategy dynamically
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
    // Similar logic as manuallyPickUpRobot, but maybe find route to charger
    auto robot = getRobotByName(robotName);
    if (!robot) return;
    Room* charger = map_->getRoomById(0);
    if (!charger) return;

    // Set path to charger
    std::vector<int> route = map_->getRoute(*robot->getCurrentRoom(), *charger);
    if (!route.empty()) {
        robot->setMovementPath(route, *map_);
    } else {
        // If no route, just pick up?
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
            alertSystem_->sendAlert("Robot " + robot->getName() + " has low battery.", "Battery");
        }
        if (robot->needsWaterRefill()) {
            alertSystem_->sendAlert("Robot " + robot->getName() + " has low water.", "Water");
        }
    }
}
