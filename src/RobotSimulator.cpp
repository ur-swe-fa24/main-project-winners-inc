#include "RobotSimulator/RobotSimulator.hpp"
#include "Robot/Robot.h"
#include "Schedular/Schedular.hpp"
#include "AlertSystem/alert_system.h"
#include "map/map.h"
#include "CleaningTask/CleaningTask.h"
#include <iostream>

RobotSimulator::RobotSimulator(std::shared_ptr<Map> map,
                               std::shared_ptr<Scheduler> scheduler,
                               std::shared_ptr<AlertSystem> alertSystem)
    : map_(map), scheduler_(scheduler), alertSystem_(alertSystem) {}

void RobotSimulator::addRobot(const std::shared_ptr<Robot>& robot) {
    robots_.push_back(robot);
}

void RobotSimulator::update(double deltaTime) {
    for (auto& robot : robots_) {
        robot->updateState(deltaTime);

        if (!robot->isCleaning() && !robot->needsCharging()) {
            auto task = scheduler_->getNextTaskForRobot(robot->getName());
            if (task) {
                robot->startCleaningTask(task);
            }
        }

        if (robot->needsCharging() && !robot->isCleaning()) {
            requestReturnToCharger(robot->getName());
        }
    }

    checkRobotStatesAndSendAlerts();
}

void RobotSimulator::assignTaskToRobot(const std::string& robotName, std::shared_ptr<CleaningTask> task) {
    for (auto& robot : robots_) {
        if (robot->getName() == robotName) {
            if (!robot->needsCharging()) {
                robot->startCleaningTask(task);
            } else {
                scheduler_->requeueTask(task);
            }
            return;
        }
    }
}

void RobotSimulator::requestReturnToCharger(const std::string& robotName) {
    auto chargerRoom = map_->getRoomById(0);
    if (!chargerRoom) return;
    for (auto& robot : robots_) {
        if (robot->getName() == robotName) {
            robot->stopCleaningTask();
            robot->moveToRoom(chargerRoom);
            robot->setCharging(true);
            return;
        }
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
