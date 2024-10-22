// permission.cpp
#include "permission/permission.h"

// Constructor implementation
Permission::Permission(const std::string &name) : name(name) {}

// Getter for name
std::string Permission::getName() const { return name; }

// Setter for name
void Permission::setName(const std::string &name) { this->name = name; }
