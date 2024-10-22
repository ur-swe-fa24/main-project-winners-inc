#include "alert_system.h"
#include "User.h"
#include "Role.h"
#include "Permission.h"
#include "Robot.h"
#include "Room.h"
#include <iostream>
#include <ctime>

// Function to simulate sending alerts
void testAlertSystem() {
    // Create Permissions
    Permission adminPermission("ADMIN");
    Permission userPermission("USER");

    // Create Roles
    Role adminRole("Admin");
    adminRole.addPermission(adminPermission);

    Role userRole("User");
    userRole.addPermission(userPermission);

    // Create Users
    User adminUser(1, "AdminUser", adminRole);
    User regularUser(2, "RegularUser", userRole);

    // Create Robot and Room instances
    Robot robot("CleaningRobot", 100);  // Example attributes
    Room room("MainRoom", 101);         // Example attributes

    // Create an alert
    std::time_t currentTime = std::time(nullptr);
    Alert cleaningAlert("Cleaning", "Robot needs maintenance", &robot, &room, currentTime);

    // Create AlertSystem
    AlertSystem alertSystem;

    // Simulate sending alerts
    std::cout << "Testing sending alert to AdminUser:" << std::endl;
    alertSystem.sendAlert(&adminUser, &cleaningAlert);

    std::cout << "\nTesting sending alert to RegularUser:" << std::endl;
    alertSystem.sendAlert(&regularUser, &cleaningAlert);
}

int main() {
    // Run the test function
    testAlertSystem();
    return 0;
}
