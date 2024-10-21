// user.h
#ifndef USER_H
#define USER_H

#include "alert.h"
#include "role.h"
#include <string>

class User {
  private:
    int id;
    std::string name;
    Role role;

  public:
    // Constructor
    User(int id, const std::string &name, const Role &role);

    // User login
    void login();

    // User logout
    void logout();

    // Receive notification (alert)
    void receiveNotification(const Alert &alert);

    // Getter for id
    int getId() const;

    // Setter for id
    void setId(int id);

    // Getter for name
    std::string getName() const;

    // Setter for name
    void setName(const std::string &name);

    // Getter for role
    Role getRole() const;

    // Setter for role
    void setRole(const Role &role);
};

#endif // USER_H
