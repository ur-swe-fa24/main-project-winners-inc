// role.h
#ifndef ROLE_H
#define ROLE_H

#include "permission/permission.h"
#include <string>
#include <vector>
#include <memory>

class Role {
private:
    std::string name;
    std::vector<Permission> permissions;

public:
    // Constructor
    Role(const std::string &name);

    // Add a permission to the role
    void addPermission(const Permission &permission);

    // Check if the role has a specific permission
    bool hasPermission(const std::string &permissionName) const;

    // Getter for name
    std::string getName() const;
};

#endif // ROLE_H
