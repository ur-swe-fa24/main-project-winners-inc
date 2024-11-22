#include "user/user.h"
#include <iostream>

// Constructor
User::User(const std::string& name, std::shared_ptr<Role> role)
    : name(name), role(role) {}

// Getter for name
std::string User::getName() const {
    return name;
}

// Getter for role
std::shared_ptr<Role> User::getRole() const {
    return role;
}

// Method to receive notifications
void User::receiveNotification(const Alert& alert) {
    // Implement notification handling
    std::cout << "User " << name << " received alert: " << alert.getMessage() << std::endl;
}
