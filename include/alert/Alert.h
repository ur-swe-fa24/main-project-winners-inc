#ifndef ALERT_H
#define ALERT_H

#include <string>
#include <ctime>  // For std::time_t
#include "Robot/Robot.h"
#include "Room/Room.h"

class Alert {
public:
    // Enum for severity level
    enum Severity {
        LOW,
        MEDIUM,
        HIGH
    };

    // Constructor and Destructor
    Alert(const std::string& type, const std::string& message, Robot* robot, Room* room, std::time_t timestamp, Severity severity = MEDIUM);
    ~Alert();

    // Getters
    std::string getType() const;
    std::string getMessage() const;
    Robot* getRobot() const;
    Room* getRoom() const;
    std::time_t getTimestamp() const;
    Severity getSeverity() const;

    // Setters
    void setMessage(const std::string& newMessage);

    // Display Alert Information
    void displayAlertInfo() const;

private:
    // Attributes
    std::string type;
    std::string message;
    Robot* robot;           // Use a pointer to the Robot class
    Room* room;             // Use a pointer to the Room class
    std::time_t timestamp;  // Timestamp of the alert
    Severity severity;      // Severity level of the alert
};

#endif // ALERT_H
