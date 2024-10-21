#include "alertSystem.h"
#include <iostream>

// sendAlert declaration
void AlertSystem::sendAlert(User* user, Alert* alert) {
    if (user != nullptr && alert != nullptr) {                              // Check that user and alert instances exist
        user->receiveNotification(alert);                                   // Pointer to specific user calling receiveNotification method defined in User class
        std::cout << "Alert sent to user: " << user->name << std::endl;     // Verify that alert was sent
    } else {                                                                // If sendAlert failed
        std::cout << "sendAlert failed. Either user or alert is nullptr." << std::endl;
    }
}
