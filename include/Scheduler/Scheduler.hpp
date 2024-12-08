#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <memory>
#include <string>
#include <vector>
#include "CleaningTask/cleaningTask.h"

class Map;
class Robot;
class RobotSimulator;
class AlertSystem;   // Forward declarations
class MongoDBAdapter;

class Scheduler {
public:
    Scheduler(Map* map, const std::vector<std::shared_ptr<Robot>>* robots)
        : map_(map), robots_(robots), taskIdCounter_(0), simulator_(nullptr), alertSystem_(nullptr), dbAdapter_(nullptr) {}

    void setSimulator(std::shared_ptr<RobotSimulator> simulator) {
        simulator_ = simulator;
    }

    // Add these two setter methods for alertSystem_ and dbAdapter_
    void setAlertSystem(std::shared_ptr<AlertSystem> alertSystem) {
        alertSystem_ = alertSystem;
    }

    void setDBAdapter(std::shared_ptr<MongoDBAdapter> dbAdapter) {
        dbAdapter_ = dbAdapter;
    }

    void addTask(std::shared_ptr<CleaningTask> task);
    std::shared_ptr<CleaningTask> getNextTaskForRobot(const std::string& robotName);
    void requeueTask(std::shared_ptr<CleaningTask> task);
    void assignCleaningTask(const std::string& robotName, int targetRoomId, const std::string& strategy);
    const std::vector<std::shared_ptr<CleaningTask>>& getAllTasks() const;

private:
    std::shared_ptr<Robot> findRobotByName(const std::string& name);
    void checkAndReturnToChargerIfNeeded(std::shared_ptr<Robot> robot);

    Map* map_;
    const std::vector<std::shared_ptr<Robot>>* robots_;
    std::vector<std::shared_ptr<CleaningTask>> tasks_;
    int taskIdCounter_;
    std::shared_ptr<RobotSimulator> simulator_;

    // Add these private members
    std::shared_ptr<AlertSystem> alertSystem_;
    std::shared_ptr<MongoDBAdapter> dbAdapter_;
};

#endif // SCHEDULER_HPP
