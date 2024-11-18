#include "schedule/schedule.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "cleaningTask/cleaningTask.h"

void Schedule::addTaskToSchedule(const std::shared_ptr<CleaningTask>& task) {
    tasks.push_back(task);
    std::cout << "Task " << task->id << " added to the schedule." << std::endl;
}

void Schedule::removeTaskFromSchedule() {
    if (!tasks.empty()) {
        std::cout << "Task " << tasks.front()->id << " removed from the schedule." << std::endl;
        tasks.erase(tasks.begin());
    } else {
        std::cout << "No tasks to dequeue." << std::endl;
    }
}


