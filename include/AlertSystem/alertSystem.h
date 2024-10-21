#ifndef ALERTSYSTEM_H
#define ALERTSYSTEM_H

#include "User.h"  // Need to define User class elsewhere
#include "Alert.h"

class AlertSystem {
public:
    // Methods
    void sendAlert(User* user, Alert* alert);  // Function to send alert

    // Constructor and Destructor
    AlertSystem() = default;
    ~AlertSystem() = default;
};

#endif // ALERTSYSTEM_H
