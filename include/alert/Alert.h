#ifndef ALERT_H
#define ALERT_H

#include <string>
#include <ctime>  // For std::time_t
#include <memory>


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
    Alert(const std::string& type, const std::string& message, std::shared_ptr<Robot> robot, std::shared_ptr<Room> room, std::time_t timestamp, Severity severity = LOW);
    ~Alert();

    // Getters
    std::string getType() const;
    std::string getMessage() const;
    std::string getTitle() const;          // New method for title
    std::string getDescription() const;    // New method for description
    std::shared_ptr<Robot> getRobot() const;
    std::shared_ptr<Room> getRoom() const;
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
    std::shared_ptr<Robot> robot;
    std::shared_ptr<Room> room;
    std::time_t timestamp;
    Severity severity;
};

#endif // ALERT_H
