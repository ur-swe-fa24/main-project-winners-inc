#ifndef ROOM_H
#define ROOM_H

#include <string>

class Room {
public:
    // Attributes
    std::string roomName;
    int roomId;
    std::string flooringType;   // Hardwood, carpet, etc.
    bool isRoomClean = true;    // true when clean, false when dirty (default to true)


    // Constructor
    Room(const std::string& roomName, int roomId);

    // Methods
    void getRoomInfo() const;   // Method to get room information (for demonstration purposes)
    void markClean(std::string roomName);   // Mark a room in the map as clean
    void markDirty(std::string roomName);   // Mark a room in the map as dirty
};

#endif // ROOM_H
