#ifndef ROBOT_H
#define ROBOT_H

#include <string>
#include <vector>    // For std::vector
#include <queue>     // For std::queue
#include "Room/Room.h"
#include "map/map.h" // For Map class
#include "CleaningTask/CleaningTask.h"

class Robot {
public:
    // Constructor
    Robot(const std::string& name, double batteryLevel, double waterLevel = 100.0);

    // Status and Updates
    void sendStatusUpdate() const;
    std::string getStatus() const;
    void update(const Map& map);

    // Battery Management
    void recharge(const Map& map);
    void depleteBattery(double amount);
    double getBatteryLevel() const;
    bool needsCharging() const;
    void startCharging();
    void stopCharging();
    bool isCharging() const;
    void setLowBatteryAlertSent(bool sent);
    bool isLowBatteryAlertSent() const;

    // Water Management
    void refillWater();
    void depleteWater(double amount);
    double getWaterLevel() const;
    bool usesWater() const;
    bool needsWaterRefill() const;
    void setLowWaterAlertSent(bool sent);
    bool isLowWaterAlertSent() const;

    // Movement and Navigation
    void setCurrentRoom(Room* room);
    Room* getCurrentRoom() const;
    Room* getNextRoom() const;
    bool moveToRoom(Room* room);
    void setMovementPath(const std::vector<int>& roomIds, const Map& map);
    double getMovementProgress() const;
    void setTargetRoom(Room* room);
    Room* getTargetRoom() const;
    bool isMoving() const;
    void updateMovement(double deltaTime);

    // Cleaning Operations
    void startCleaning();
    void stopCleaning();
    bool isCleaning() const;
    void updateCleaning(double deltaTime);
    void startCleaning(CleaningTask::CleanType cleaningType);

    // Task Management
    void addTaskToQueue(const std::shared_ptr<CleaningTask>& task);
    const std::queue<std::shared_ptr<CleaningTask>>& getTaskQueue() const;
    bool hasPendingTasks() const;
    void saveCurrentTask();
    void resumeSavedTask();
    void clearTaskQueue();

    // Maintenance
    bool needsMaintenance() const;

    // Utility
    std::string getName() const;
    static std::string cleaningStrategyToString(CleaningTask::CleanType cleanType);

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
    std::shared_ptr<CleaningTask> savedTask_;  // For saving interrupted tasks

    // Helper functions for update
    void updateBatteryStatus(double deltaTime);
    void updateWaterStatus(double deltaTime);
    void updateTaskProgress(double deltaTime);
    void handleLowResources();
};

#endif // ROBOT_H
