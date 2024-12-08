#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <memory>
#include <string>
#include <vector>
#include "CleaningTask/cleaningTask.h"

class Map;
class Robot;
class Scheduler {
public:
    Scheduler(Map* map, const std::vector<std::shared_ptr<Robot>>* robots) 
        : map_(map), robots_(robots), taskIdCounter_(0) {}

    void addTask(std::shared_ptr<CleaningTask> task);
    std::shared_ptr<CleaningTask> getNextTaskForRobot(const std::string& robotName);
    void requeueTask(std::shared_ptr<CleaningTask> task);
    void assignCleaningTask(const std::string& robotName, int targetRoomId, const std::string& strategy);
    const std::vector<std::shared_ptr<CleaningTask>>& getAllTasks() const;

    std::shared_ptr<Robot> findRobotByName(const std::string& name);

private:
    Map* map_;
    const std::vector<std::shared_ptr<Robot>>* robots_;
    std::vector<std::shared_ptr<CleaningTask>> tasks_;
    int taskIdCounter_;
};

#endif // SCHEDULER_HPP
