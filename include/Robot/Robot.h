#ifndef ROBOT_H
#define ROBOT_H

#include <string>
#include <vector>    // For std::vector
#include <queue>     // For std::queue
#include "Room/Room.h"
#include "map/map.h" // For Map class

class Robot {
public:
    // Constructor
    Robot(const std::string& name, int batteryLevel);

    // Status update method
    void sendStatusUpdate() const;

    // Methods to manage battery
    void recharge();
    void depleteBattery(int amount);

    // Methods to perform actions
    void startCleaning();
    void stopCleaning();
    bool isCleaning() const;

    // Maintenance and alerts
    bool needsMaintenance() const;
    void setLowBatteryAlertSent(bool sent);
    bool isLowBatteryAlertSent() const;

    // Getters
    std::string getName() const;
    int getBatteryLevel() const;

    void setCurrentRoom(Room* room);
    Room* getCurrentRoom() const;
    bool moveToRoom(Room* room);  // Move to an adjacent room

    void setMovementPath(const std::vector<int>& roomIds, Map& map);
    void update();  // Update robot status (move along path, deplete battery)

private:
    // Attributes
    std::string name;
    int batteryLevel;
    bool cleaning_;
    bool lowBatteryAlertSent_;
    Room* currentRoom_;
    std::queue<Room*> movementQueue_;
};

#endif // ROBOT_H
