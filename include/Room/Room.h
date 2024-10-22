// Room.h
#ifndef ROOM_H
#define ROOM_H

#include <string>

class Room {
public:
    // Attributes
    std::string roomName;
    int roomId;

    // Constructor
    Room(const std::string& roomName, int roomId);

    // Method to get room information (for demonstration purposes)
    void getRoomInfo() const;
};

#endif // ROOM_H
