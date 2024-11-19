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
    bool isRoomClean;           // true when clean, false when dirty (default to true)
    std::vector<Room*> neighbors;

    // Constructor
    Room(const std::string& roomName, int roomId, const std::string& flooringType = "", bool isRoomClean = true, const std::vector<Room*>& neighbors = std::vector<Room*>());

    // Getter methods
    std::string getRoomName() const;
    int getRoomId() const;

    // Methods
    void getRoomInfo() const;
    void markClean();   // Mark a room as clean
    void markDirty();   // Mark a room as dirty
    void addNeighbor(Room* neighbor);       // Add a neighbor room

    // Destructor (optional)
    ~Room() = default;
};

#endif // ROOM_H