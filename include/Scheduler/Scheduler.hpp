#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <memory>
#include <string>
#include <vector>
#include "CleaningTask/cleaningTask.h"

class Map;
class Robot;
class RobotSimulator;

class Scheduler {
public:
    // Accept const pointer to robot vector
    Scheduler(Map* map, const std::vector<std::shared_ptr<Robot>>* robots)
        : map_(map), robots_(robots), taskIdCounter_(0), simulator_(nullptr) {}

    void setSimulator(std::shared_ptr<RobotSimulator> simulator) {
        simulator_ = simulator;
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
};

#endif // SCHEDULER_HPP

