// src/Room.cpp
#include "Room/Room.h"
#include <iostream>

// Constructor implementation
Room::Room(const std::string& roomName, int roomId)
    : roomName(roomName), roomId(roomId) {
    // Initialization code if needed
}

// Implement getRoomInfo method (if used)
void Room::getRoomInfo() const {
    std::cout << "Room Name: " << roomName << ", Room ID: " << roomId << std::endl;
}

