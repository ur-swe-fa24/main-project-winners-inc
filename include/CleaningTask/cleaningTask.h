#ifndef CLEANINGTASK_H
#define CLEANINGTASK_H

#include <string>
#include <memory>

// Forward declarations
class Robot;
class Room;

class CleaningTask {
public:
    // Enum for priority level
    enum Priority { LOW, MEDIUM, HIGH };

    // Enum for cleaning type
    enum CleanType { VACUUM, SCRUB, SHAMPOO };

    // Constructor
    CleaningTask(int id, Priority priority, CleanType cleaningType, Room* room);
    CleaningTask(Room* room, CleanType cleaningType); 
    ~CleaningTask() = default;

    // Getters
    int getID() const;
    Priority getPriority() const;
    std::string getStatus() const;
    CleanType getCleanType() const;
    // std::shared_ptr<Room> getRoom() const;

    Room* getRoom() const;

    std::shared_ptr<Robot> getRobot() const;

    // Methods for assigning a task and marking task statuses
    void assignRobot(const std::shared_ptr<Robot>& robot);
    void markCompleted();
    void markFailed();

    // Helper function to convert string to CleanType
    static CleanType stringToCleanType(const std::string& str) {
        if (str == "Vacuum") return VACUUM;
        if (str == "Scrub") return SCRUB;
        if (str == "Shampoo") return SHAMPOO;
        return VACUUM; // Default
    }

    void setStatus(const std::string& newStatus);


private:
    int id;
    Priority priority;              // Priority level
    std::string status;            // "Pending", "In Progress", "Completed", "Failed"
    CleanType cleaningType;        // Type of cleaning
    // std::shared_ptr<Room> room;
    Room* room;
    std::shared_ptr<Robot> robot;
};

#endif // CLEANINGTASK_H
