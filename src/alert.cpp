#include "Alert.h"

// Constructor
Alert::Alert(const std::string& type, const std::string& message, const Robot& robot, const Room& room, std::time_t timestamp)
    : type(type), message(message), robot(robot), room(room), timestamp(timestamp) {}

// Destructor
Alert::~Alert() {}
