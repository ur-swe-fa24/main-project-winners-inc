// role.cpp
#include "role/role.h"

// Constructor implementation
Role::Role(const std::string &name) : name(name) {}

// Add a permission to the role
void Role::addPermission(const Permission &permission) {
    permissions.push_back(permission);
}

// Check if the role has a specific permission
bool Role::hasPermission(const std::string &permissionName) const {
    for (const auto &perm : permissions) {
        if (perm.getName() == permissionName) {
            return true;
        }
    }
    return false;
}

// Getter for name
std::string Role::getName() const {
    return name;
}
