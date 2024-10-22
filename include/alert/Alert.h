#ifndef ALERT_H
#define ALERT_H

#include <string>
// #include <ctime>  // For DateTime
#include "Robot.h"
#include "Room.h"

class Alert {
public:
    // Attributes
    std::string type;
    std::string message;
    Robot robot;                // Need to define Robot class elsewhere
    Room room;                  // Need to define Room class elsewhere
    //std::time_t timestamp;    // Do we need DateTime class?

    // Constructor and destructor
    Alert(const std::string& type, const std::string& message, Robot* robot, Room* room, std::time_t timestamp);
    ~Alert();
};

#endif // ALERT_H
