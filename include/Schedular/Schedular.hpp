#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <memory>
#include <string>
#include <vector>
#include "CleaningTask/CleaningTask.h"
#include "Room/Room.h"

class Scheduler {
public:
    void addTask(std::shared_ptr<CleaningTask> task);
    std::shared_ptr<CleaningTask> getNextTaskForRobot(const std::string& robotName);
    void requeueTask(std::shared_ptr<CleaningTask> task);

    void assignCleaningTask(const std::string& robotName, int targetRoomId, const std::string& strategy);
    const std::vector<std::shared_ptr<CleaningTask>>& getAllTasks() const;

private:
    std::vector<std::shared_ptr<CleaningTask>> tasks_;
};

#endif // SCHEDULER_HPP
