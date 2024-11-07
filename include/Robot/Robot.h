#ifndef ROBOT_H
#define ROBOT_H

#include <string>

class Robot {
public:
    // Constructor
    Robot(const std::string& name, int batteryLevel);

    // Status update method (for demonstration purposes)
    void sendStatusUpdate() const;

    // Method to recharge the robot
    void recharge();

    // Method to perform a maintenance check
    bool needsMaintenance() const;

    // Getters
    std::string getName() const;
    int getBatteryLevel() const;

    // Method to deplete battery (for testing purposes)
    void depleteBattery(int amount);

private:
    // Attributes
    std::string name;
    int batteryLevel;
};

#endif // ROBOT_H
