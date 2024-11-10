#include "Room/Room.h"
#include <iostream>

// Constructor implementation
Room::Room(const std::string& roomName, int roomId, const std::string& flooringType, bool isRoomClean)
    : roomName(roomName), roomId(roomId), flooringType(flooringType), isRoomClean(isRoomClean) {}

// Getter for room name
std::string Room::getRoomName() const {
    return roomName;
}

// Getter for room ID
int Room::getRoomId() const {
    return roomId;
}

// Method to get room information
void Room::getRoomInfo() const {
    std::cout << "Room Name: " << roomName
              << ", Room ID: " << roomId
              << ", Flooring Type: " << flooringType
              << ", Room is clean?: " << (isRoomClean ? "Yes" : "No") << std::endl;
}

// Marking a given room as clean
void Room::markClean(std::string roomName){
    isRoomClean = true;
}

// Marking a given room as dirty
void Room::markDirty(std::string roomName){
    isRoomClean = false;
}

// Adding a neighbor to neighbors vector
void Room::addNeighbor(Room* neighbor){
    neighbors.push_back(neighbor);
}
