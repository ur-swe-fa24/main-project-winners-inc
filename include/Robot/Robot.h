// Robot.h
#ifndef ROBOT_H
#define ROBOT_H

#include <string>

class Robot {
public:
    // Attributes
    std::string name;
    int batteryLevel;

    // Constructor
    Robot(const std::string& name, int batteryLevel);

    // Method to send a status update (for demonstration purposes)
    void sendStatusUpdate() const;
};

#endif // ROBOT_H
