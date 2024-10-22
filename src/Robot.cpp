// Robot.cpp
#include "Robot.h"
#include <iostream>

// Constructor implementation
Robot::Robot(const std::string& name, int batteryLevel) : name(name), batteryLevel(batteryLevel) {}

// Method to send a status update
void Robot::sendStatusUpdate() const {
    std::cout << "Robot " << name << " has " << batteryLevel << "% battery remaining." << std::endl;
}
