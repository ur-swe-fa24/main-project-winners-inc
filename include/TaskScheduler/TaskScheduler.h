#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include <queue>
#include <memory>
#include <mutex>
#include "CleaningTask/cleaningTask.h"

class TaskScheduler {
public:
    // Singleton instance getter
    static TaskScheduler& getInstance();

    // Delete copy constructor and assignment operator
    TaskScheduler(const TaskScheduler&) = delete;
    TaskScheduler& operator=(const TaskScheduler&) = delete;

    // Enqueue a new task with priority
    void enqueueTask(std::shared_ptr<CleaningTask> task);

    // Dequeue the highest priority task
    std::shared_ptr<CleaningTask> dequeueTask();

    // Check if there are any tasks in the queue
    bool hasTasks() const;

    // Get number of tasks in queue
    size_t taskCount() const;

private:
    // Private constructor for singleton
    TaskScheduler() = default;

    // Custom comparator for the priority queue
    struct TaskComparator {
        bool operator()(const std::shared_ptr<CleaningTask>& a, const std::shared_ptr<CleaningTask>& b) const {
            // Higher priority tasks should come first
            return a->getPriority() < b->getPriority();
        }
    };

    // Priority queue to store tasks
    std::priority_queue<std::shared_ptr<CleaningTask>, 
                       std::vector<std::shared_ptr<CleaningTask>>, 
                       TaskComparator> taskQueue;

    // Mutex for thread safety
    mutable std::mutex mutex;
};

#endif // TASKSCHEDULER_H
