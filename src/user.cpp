// user.cpp
#include "user/user.h"
#include <iostream>

// Constructor implementation
User::User(int id, const std::string &name, const Role &role) : id(id), name(name), role(role) {}

// User login
void User::login() { std::cout << name << " has logged in." << std::endl; }

// User logout
void User::logout() { std::cout << name << " has logged out." << std::endl; }

// Receive notification (alert)
void User::receiveNotification(const Alert &alert) {
    std::cout << "Alert for " << name << ": " << alert.message << std::endl;
}

// Getter for id
int User::getId() const { return id; }

// Setter for id
void User::setId(int id) { this->id = id; }

// Getter for name
std::string User::getName() const { return name; }

// Setter for name
void User::setName(const std::string &name) { this->name = name; }

// Getter for role
Role User::getRole() const { return role; }

// Setter for role
void User::setRole(const Role &role) { this->role = role; }
