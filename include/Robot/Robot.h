#ifndef ROBOT_H
#define ROBOT_H

#include <string>
#include "Room/Room.h"

class Robot {
public:
    // Constructor
    Robot(const std::string& name, int batteryLevel);

    // Status update method (for demonstration purposes)
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

private:
    // Attributes
    std::string name;
    int batteryLevel;
    bool cleaning_;
    bool lowBatteryAlertSent_;
    Room* currentRoom_;
};

#endif // ROBOT_H
