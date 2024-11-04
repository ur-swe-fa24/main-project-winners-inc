#ifndef ROOM_H
#define ROOM_H

#include <string>

class Room {
public:
    // Constructor
    Room(const std::string& roomName, int roomId);

    // Getters
    std::string getRoomName() const;
    int getRoomId() const;

    // Method to get room information (for demonstration purposes)
    void getRoomInfo() const;

    // Setter to update the room name
    void setRoomName(const std::string& newRoomName);

    // Utility method to simulate checking occupancy status
    bool isOccupied() const;

private:
    // Attributes
    std::string roomName;
    int roomId;
    bool occupied; // Example attribute to track occupancy
};

#endif // ROOM_H
