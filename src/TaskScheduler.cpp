#include "TaskScheduler/TaskScheduler.h"
#include <iostream>

TaskScheduler& TaskScheduler::getInstance() {
    static TaskScheduler instance;
    return instance;
}

void TaskScheduler::enqueueTask(std::shared_ptr<CleaningTask> task) {
    std::lock_guard<std::mutex> lock(mutex);
    taskQueue.push(task);
    std::cout << "[TaskScheduler] Enqueued task with priority: " 
              << static_cast<int>(task->getPriority()) << std::endl;
}

std::shared_ptr<CleaningTask> TaskScheduler::dequeueTask() {
    std::lock_guard<std::mutex> lock(mutex);
    if (taskQueue.empty()) {
        return nullptr;
    }
    
    auto task = taskQueue.top();
    taskQueue.pop();
    std::cout << "[TaskScheduler] Dequeued task with priority: " 
              << static_cast<int>(task->getPriority()) << std::endl;
    return task;
}

bool TaskScheduler::hasTasks() const {
    std::lock_guard<std::mutex> lock(mutex);
    return !taskQueue.empty();
}

size_t TaskScheduler::taskCount() const {
    std::lock_guard<std::mutex> lock(mutex);
    return taskQueue.size();
}
