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
    std::string getStatus() const;
    int getMovementProgress() const;
    Room* getNextRoom() const;



    void setCurrentRoom(Room* room);
    Room* getCurrentRoom() const;
    bool moveToRoom(Room* room);  // Move to an adjacent room

    void setMovementPath(const std::vector<int>& roomIds, const Map& map); // Changed Map& to const Map&
    // void update();  // Update robot status (move along path, deplete battery)
    void startCharging();
    void stopCharging();
    bool isCharging() const;

    void setTargetRoom(Room* room);
    void update(const Map& map);  // Modify this line



private:
    // Attributes
    std::string name;
    int batteryLevel;
    bool cleaning_;
    bool lowBatteryAlertSent_;
    Room* currentRoom_;
    std::queue<Room*> movementQueue_;
    Room* nextRoom_; // The room the robot is moving towards

    bool isCharging_;
    int chargingTimeRemaining_; // in seconds
    double movementProgress_; // Time remaining to move to next room

    double cleaningTimeRemaining_; // Time remaining to clean the room


    Room* targetRoom_; // The room the robot is assigned to clean


};

#endif // ROBOT_H
