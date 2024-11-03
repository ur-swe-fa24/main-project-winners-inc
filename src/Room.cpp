// src/room.cpp
#include "room/room.h"
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

// Marking a given room as clean
void Room::markClean(std::string roomName){
    isRoomClean == true;
}

// Marking a given room as dirty
void Room::markDirty(std::string roomName){
    isRoomClean == false;
}
