#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <vector>

class Room {
public:
    // Attributes
    std::string roomName;
    int roomId;
    std::string flooringType;   // Hardwood, carpet, etc.
    bool isRoomClean = true;    // true when clean, false when dirty (default to true)
    std::vector<Room*> neighbors;

    // Constructor
    Room(const std::string& roomName, int roomId, const std::string& flooringType, bool isRoomClean);

    // Methods
    void getRoomInfo() const;   // Method to get room information (for demonstration purposes)
    void markClean(std::string roomName);   // Mark a room in the map as clean
    void markDirty(std::string roomName);   // Mark a room in the map as dirty
    void addNeighbor(Room* neighbor);       // Used to create vector of neighbors for map purposes
};

#endif // ROOM_H
