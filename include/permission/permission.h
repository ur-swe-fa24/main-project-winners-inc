// permission.h
#ifndef PERMISSION_H
#define PERMISSION_H

#include <string>

class Permission {
private:
    std::string name;

public:
    // Constructor
    Permission(const std::string &name);

    // Getter for name
    std::string getName() const;
};

#endif // PERMISSION_H
