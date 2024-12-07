#ifndef ROBOT_H
#define ROBOT_H

#include <string>
#include <memory>

class Room;
class CleaningTask;

class Robot {
public:
    Robot(const std::string& name, double batteryLevel, double waterLevel = 100.0);

    void updateState(double deltaTime);
    void startCleaningTask(std::shared_ptr<CleaningTask> task);
    void stopCleaningTask();
    void moveToRoom(Room* targetRoom);

    double getBatteryLevel() const;
    double getWaterLevel() const;
    bool needsCharging() const;
    bool needsWaterRefill() const;

    bool isCleaning() const;
    bool isMoving() const;
    Room* getCurrentRoom() const;
    std::string getName() const;

    void setCurrentRoom(Room* room);
    void setCharging(bool charging);
    void refillWater();
    void fullyRecharge();

    // Added methods for MongoDBAdapter
    bool isCharging() const;
    double getMovementProgress() const;
    bool needsMaintenance() const;
    bool isLowBatteryAlertSent() const;
    bool isLowWaterAlertSent() const;
    std::string getStatus() const; // "Cleaning", "Idle", "Moving", "Charging"

    void setLowBatteryAlertSent(bool val);
    void setLowWaterAlertSent(bool val);

private:
    std::string name_;
    double batteryLevel_;
    double waterLevel_;
    bool cleaning_;
    bool isCharging_;
    double cleaningProgress_;
    double movementProgress_;
    Room* currentRoom_;
    std::shared_ptr<CleaningTask> currentTask_;

    bool lowBatteryAlertSent_;
    bool lowWaterAlertSent_;
};

#endif // ROBOT_H
