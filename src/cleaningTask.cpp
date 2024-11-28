#include "cleaningTask/cleaningTask.h"
#include "Schedule/schedule.h"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include <string>
#include <iostream>
#include <vector>
#include <memory>

// Constructor
CleaningTask::CleaningTask(int id, Priority priority, CleanType cleaningType, Room* room)
    : id(id), priority(priority), cleaningType(cleaningType), room(room) {}

// Assign a robot with a task
void CleaningTask::assignRobot(const std::shared_ptr<Robot>& robot) {
    this->robot = robot;
    status = "In Progress";
    std::cout << "Robot " << robot->getName() << " assigned to task " << id << "." << std::endl;
}

// Methods to mark clean as either completed or failed
void CleaningTask::markCompleted() {
    status = "Completed";
    std::cout << "Task " << id << " marked as completed." << std::endl;
}
void CleaningTask::markFailed() {
    status = "Failed";
    std::cout << "Task " << id << " marked as failed." << std::endl;
}


// Getter declarations
int CleaningTask::getID() const {
    return id;
}

CleaningTask::Priority CleaningTask::getPriority() const {
    return priority;
}

std::string CleaningTask::getStatus() const{
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
