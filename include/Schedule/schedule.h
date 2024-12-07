#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "CleaningTask/CleaningTask.h"


class Schedule {
public:
    // Vector of tasks to maintain an ordered list of tasks
    std::vector<std::shared_ptr<CleaningTask>> tasks;

    void addTaskToSchedule(const std::shared_ptr<CleaningTask>& task) {}
    void removeTaskFromSchedule() {}
};

#endif // SCHEDULE_H