#include "alert/Alert.h"
#include <iostream>

// Constructor
Alert::Alert(const std::string& type, const std::string& message, Robot* robot, Room* room, std::time_t timestamp, Severity severity)
    : type(type), message(message), robot(robot), room(room), timestamp(timestamp), severity(severity) {}

// Destructor
Alert::~Alert() {}

// Getter for alert type (title)
std::string Alert::getTitle() const {
    return type;
}

// In Alert.cpp
std::string Alert::getMessage() const {
    return message;
}

// Getter for alert message (description)
std::string Alert::getDescription() const {
    return message;
}

// Getter for robot pointer
Robot* Alert::getRobot() const {
    return robot;
}

// Getter for room pointer
Room* Alert::getRoom() const {
    return room;
}

// Getter for alert timestamp
std::time_t Alert::getTimestamp() const {
    return timestamp;
}

// Getter for severity
Alert::Severity Alert::getSeverity() const {
    return severity;
}

// Setter to update the alert message
void Alert::setMessage(const std::string& newMessage) {
    message = newMessage;
}

// Display alert information
void Alert::displayAlertInfo() const {
    std::cout << "Alert Type: " << type << std::endl;
    std::cout << "Message: " << message << std::endl;
    std::cout << "Robot: " << (robot ? robot->getName() : "None") << std::endl;
    std::cout << "Room: " << (room ? room->getRoomName() : "None") << std::endl;
    std::cout << "Timestamp: " << std::ctime(&timestamp);
    std::cout << "Severity: " << (severity == HIGH ? "High" : (severity == MEDIUM ? "Medium" : "Low")) << std::endl;
}
