#ifndef ROBOT_H
#define ROBOT_H

#include <string>
#include <vector>    // For std::vector
#include <queue>     // For std::queue
#include "Room/Room.h"
#include "map/map.h" // For Map class
#include "cleaningTask/cleaningTask.h"

class Robot {
public:
    // Constructor
    Robot(const std::string& name, double batteryLevel, double waterLevel = 100.0);

    // Status update method
    void sendStatusUpdate() const;

    // Methods to manage battery
    void recharge(const Map& map);           // Updated declaration
    void depleteBattery(double amount);

    // Methods to manage water
    void refillWater();
    void depleteWater(double amount);
    double getWaterLevel() const;
    bool needsWaterRefill() const;

    // Methods to perform actions
    void startCleaning();
    void stopCleaning();
    bool isCleaning() const;

    // Maintenance and alerts
    bool needsMaintenance() const;
    void setLowBatteryAlertSent(bool sent);
    bool isLowBatteryAlertSent() const;
    void setLowWaterAlertSent(bool sent);
    bool isLowWaterAlertSent() const;

    // Getters
    std::string getName() const;
    double getBatteryLevel() const;
    std::string getStatus() const;
    double getMovementProgress() const;
    Room* getNextRoom() const;

    void setCurrentRoom(Room* room);
    Room* getCurrentRoom() const;
    bool moveToRoom(Room* room);  // Move to an adjacent room

    void setMovementPath(const std::vector<int>& roomIds, const Map& map); 
    void startCharging();
    void stopCharging();
    bool isCharging() const;

    void setTargetRoom(Room* room);
    void update(const Map& map);  

    void addTaskToQueue(const std::shared_ptr<CleaningTask>& task);
    void startCleaning(CleaningTask::CleanType cleaningType);

    std::queue<std::shared_ptr<CleaningTask>> getTaskQueue() const;

    // Helper method to convert CleanType to string
    static std::string cleaningStrategyToString(CleaningTask::CleanType cleanType) {
        switch (cleanType) {
            case CleaningTask::VACUUM: return "Vacuum";
            case CleaningTask::SCRUB: return "Scrub";
            case CleaningTask::SHAMPOO: return "Shampoo";
            default: return "Unknown";
        }
    }

private:
    // Attributes
    std::string name;
    double batteryLevel;
    double waterLevel_;  // New water level property
    bool cleaning_;
    bool lowBatteryAlertSent_;
    bool lowWaterAlertSent_;  // New low water alert flag
    Room* currentRoom_;
    std::queue<Room*> movementQueue_;
    Room* nextRoom_; // The room the robot is moving towards

    bool isCharging_;
    double chargingTimeRemaining_; // in seconds
    double movementProgress_; // Time remaining to move to next room

    double cleaningTimeRemaining_; // Time remaining to clean the room


    Room* targetRoom_; // The room the robot is assigned to clean

    // Add this member variable
    std::queue<std::shared_ptr<CleaningTask>> taskQueue_;

    // Add a flag to check if the robot is returning to the charger
    bool returningToCharger_ = false;

    // Add a flag to indicate if the robot needs to resume tasks
    bool hasPendingTasks_ = false;

    std::shared_ptr<CleaningTask> currentCleaningTask_;



};

#endif // ROBOT_H
