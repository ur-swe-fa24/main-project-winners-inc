#include "CleaningTask/cleaningTask.h"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include <iostream>

CleaningTask::CleaningTask(int id, Priority priority, CleanType cleaningType, Room* room)
    : id(id), priority(priority), status("Pending"), cleaningType(cleaningType), room(room), robot(nullptr) {}

void CleaningTask::assignRobot(const std::shared_ptr<Robot>& robot) {
    this->robot = robot;
    // Keep it Pending until robot actually starts cleaning:
    status = "Pending"; 
    std::cout << "[DEBUG] Task " << id << " assigned to " << robot->getName() << " and is now Pending.\n";
}

void CleaningTask::markCompleted() {
    status = "Completed";
    std::cout << "[DEBUG] Task " << id << " marked as completed.\n";
}

void CleaningTask::markFailed() {
    status = "Failed";
    std::cout << "[DEBUG] Task " << id << " marked as failed.\n";
}

void CleaningTask::setStatus(const std::string& newStatus) {
    std::cout << "[DEBUG] Task " << id << " status changing from " << status << " to " << newStatus << "\n";
    status = newStatus;
}

int CleaningTask::getID() const {
    return id;
}

CleaningTask::Priority CleaningTask::getPriority() const {
    return priority;
}

std::string CleaningTask::getStatus() const {
    return status;
}

CleaningTask::CleanType CleaningTask::getCleanType() const {
    return cleaningType;
}

Room* CleaningTask::getRoom() const {
    return room;
}

std::shared_ptr<Robot> CleaningTask::getRobot() const {
    return robot;
}
