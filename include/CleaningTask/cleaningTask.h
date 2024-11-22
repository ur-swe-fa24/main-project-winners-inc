#ifndef CLEANINGTASK_H
#define CLEANINGTASK_H

#include "robot/Robot.h"
#include "room/Room.h"
#include <string>


class CleaningTask {
    public:
    int id;
    // Enum for priority level
    enum Priority { LOW, MEDIUM, HIGH };

    // Enum for cleaning type
    enum CleanType { Scrub, Mop, Vacuum , SpotTreat};

    // Constructor and Destructor methods
    ~CleaningTask();
    CleaningTask(int id, Priority priority, CleanType cleaningType, std::shared_ptr<Room> room) {}

    // Getters
    int getID() const;
    Priority getPriority() const;
    std::string getStatus() const;
    CleanType getCleanType() const;
    std::shared_ptr<Room> getRoom() const;
    std::shared_ptr<Robot> getRobot() const;


    // Methods for assigning a task and marking task statuses
    void assignRobot(int robotID) {}
    void markCompleted() {}
    void markFailed() {}


    private:
    // Attributes
    Priority priority;              // Enum above for different priority levels
    std::string status;             // Potential statuses may be "Pending", "In Progress", "Completed", "Failed"
    CleanType cleaningType;         // Enum above for different CleanTypes
    std::shared_ptr<Room> room;
    std::shared_ptr<Robot> robot;
};

#endif // CLEANINGTASK_H
