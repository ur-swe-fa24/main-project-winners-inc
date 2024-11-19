#include "Room/Room.h"
#include <iostream>

// Constructor implementation
Room::Room(const std::string& roomName, int roomId, const std::string& flooringType, bool isRoomClean, const std::vector<Room*>& neighbors)
    : roomName(roomName), roomId(roomId), flooringType(flooringType), isRoomClean(isRoomClean), neighbors(neighbors) {}

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
void Room::markClean(){
    isRoomClean = true;
}

// Marking a given room as dirty
void Room::markDirty(){
    isRoomClean = false;
}

// Adding a neighbor to neighbors vector
void Room::addNeighbor(Room* neighbor){
    neighbors.push_back(neighbor);
}