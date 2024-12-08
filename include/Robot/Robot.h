#ifndef ROBOT_H
#define ROBOT_H

#include <string>
#include <memory>
#include <queue>
#include "CleaningTask/cleaningTask.h"
#include "Room/Room.h"
#include "map/map.h"

class Robot {
public:
    // Enums for size and cleaning strategy
    enum class Size { SMALL, MEDIUM, LARGE };
    enum class Strategy { VACUUM, SCRUB, SHAMPOO };

    // Modified constructor to accept size and strategy
    Robot(const std::string& name, double batteryLevel, Size size, Strategy strategy, double waterLevel = 100.0);

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

    void saveCurrentTask();
    bool resumeSavedTask();
    void setMap(Map* m) { robotMap_ = m; }

    // Getters for size and strategy if needed
    Size getSize() const { return size_; }
    Strategy getStrategy() const { return strategy_; }

    int errorCount_;
    double totalWorkTime_; // total cleaning/working time in seconds
    bool failed_;          // indicates if the robot has failed
    int getErrorCount() const { return errorCount_; }
    double getTotalWorkTime() const { return totalWorkTime_; }
    bool isFailed() const { return failed_; }


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

    Map* robotMap_;

    std::queue<Room*> movementQueue_;
    std::shared_ptr<CleaningTask> currentTask_;
    std::shared_ptr<CleaningTask> savedTask_;
    double savedCleaningTimeRemaining_;

    Size size_;
    Strategy strategy_;
};

#endif // ROBOT_H
