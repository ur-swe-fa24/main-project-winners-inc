#include "Room/Room.h"
#include <iostream>

// Constructor implementation
Room::Room(const std::string& roomName, int roomId)
    : roomName(roomName), roomId(roomId), occupied(false) {
    // Initialization code if needed
}

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
    std::cout << "Room Name: " << roomName << ", Room ID: " << roomId << std::endl;
}

// Setter to update the room name
void Room::setRoomName(const std::string& newRoomName) {
    roomName = newRoomName;
    std::cout << "Room name updated to: " << roomName << std::endl;
}

// Method to check occupancy status
bool Room::isOccupied() const {
    return occupied;
}

// Optional: A method to update occupancy status (not requested, but could be useful)
// void Room::setOccupied(bool status) {
//     occupied = status;
//     std::cout << "Room " << roomName << " occupancy status updated to: " << (occupied ? "Occupied" : "Vacant") << std::endl;
// }
