#ifndef ROBOT_H
#define ROBOT_H

#include <string>
#include <memory>
#include <queue>
#include "CleaningTask/cleaningTask.h"
#include "Room/Room.h"

class Map;
class CleaningTask;

class Robot {
public:
    Robot(const std::string& name, double batteryLevel, double waterLevel = 100.0);

    void updateState(double deltaTime);
    void startCleaning(CleaningTask::CleanType cleaningType);
    void stopCleaning();
    void setMovementPath(const std::vector<int>& roomIds, const Map& map);
    void moveToRoom(Room* room);

    double getBatteryLevel() const;
    double getWaterLevel() const;
    bool needsCharging() const;
    bool needsWaterRefill() const;
    bool isCleaning() const;
    bool isMoving() const;
    Room* getCurrentRoom() const;
    Room* getNextRoom() const;
    std::string getName() const;
    void setCurrentRoom(Room* room);
    void setCharging(bool charging);
    void refillWater();
    void fullyRecharge();

    bool isCharging() const;
    double getMovementProgress() const;
    bool needsMaintenance() const;
    bool isLowBatteryAlertSent() const;
    bool isLowWaterAlertSent() const;
    std::string getStatus() const;

    void setLowBatteryAlertSent(bool val);
    void setLowWaterAlertSent(bool val);
    void setCurrentTask(std::shared_ptr<CleaningTask> task);
    std::shared_ptr<CleaningTask> getCurrentTask() const;
    void setTargetRoom(Room* room);

private:
    std::string name_;
    double batteryLevel_;
    double waterLevel_;
    bool cleaning_;
    bool isCharging_;
    double cleaningProgress_;
    double movementProgress_;
    Room* currentRoom_;
    Room* nextRoom_;
    double cleaningTimeRemaining_;
    Room* targetRoom_;
    bool lowBatteryAlertSent_;
    bool lowWaterAlertSent_;

    std::queue<Room*> movementQueue_;
    std::shared_ptr<CleaningTask> currentTask_;
};

#endif // ROBOT_H
