#include "Schedular/Schedular.hpp"
#include <algorithm>
#include <iostream>

void Scheduler::addTask(std::shared_ptr<CleaningTask> task) {
    tasks_.push_back(task);
}

std::shared_ptr<CleaningTask> Scheduler::getNextTaskForRobot(const std::string& /*robotName*/) {
    if (tasks_.empty()) return nullptr;
    auto task = tasks_.front();
    tasks_.erase(tasks_.begin());
    return task;
}

void Scheduler::requeueTask(std::shared_ptr<CleaningTask> task) {
    tasks_.push_back(task);
}

void Scheduler::assignCleaningTask(const std::string& robotName, int targetRoomId, const std::string& strategy) {
    // Create a new cleaning task based on strategy and room
    // For now we assume you have a method to get a room pointer by ID.
    // The calling code ensures the room pointer is known.
    CleaningTask::CleanType ctype = CleaningTask::stringToCleanType(strategy);
    // Just create a dummy Room* or in reality get from Map
    // This is just an example, adjust as needed where you have Map access.
    Room* targetRoom = nullptr; // You must set this from outside or pass it in.
    auto newTask = std::make_shared<CleaningTask>(1, CleaningTask::MEDIUM, ctype, targetRoom);
    addTask(newTask);
    std::cout << "Assigned task to robot " << robotName << " to clean room " << targetRoomId 
              << " using strategy " << strategy << "." << std::endl;
}

const std::vector<std::shared_ptr<CleaningTask>>& Scheduler::getAllTasks() const {
    return tasks_;
}
