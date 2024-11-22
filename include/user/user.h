#ifndef USER_H
#define USER_H

#include <string>
#include <memory>
#include "role/role.h"
#include "alert/Alert.h"

class User {
private:
    std::string name;
    std::shared_ptr<Role> role;

public:
    // Constructor
    User(const std::string& name, std::shared_ptr<Role> role);

    // Getters
    std::string getName() const;
    std::shared_ptr<Role> getRole() const;

    // Notification method
    void receiveNotification(const Alert& alert);
};

#endif // USER_H
