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

    // Setter for name
    void setName(const std::string &name);
};

#endif // PERMISSION_H
