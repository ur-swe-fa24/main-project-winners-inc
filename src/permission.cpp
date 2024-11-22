// permission.cpp
#include "permission/permission.h"

// Constructor implementation
Permission::Permission(const std::string &name) : name(name) {}

// Getter for name
std::string Permission::getName() const {
    return name;
}
