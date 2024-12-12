#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <vector>

// Forward declaration
class Map;

class Room {
public:
    // Attributes
    std::string roomName;
    int roomId;
    std::string flooringType;   // Hardwood, carpet, etc.
    bool isRoomClean;           // true when clean, false when dirty (default to true)
    std::string size;           // small, medium, or large
    std::vector<Room*> neighbors;
    Map* map;  // Pointer to the map this room belongs to

    // Constructor
    Room(const std::string& roomName, int roomId, const std::string& flooringType = "", 
         const std::string& size = "medium", bool isRoomClean = true, 
         const std::vector<Room*>& neighbors = std::vector<Room*>());

    // Getter methods
    std::string getRoomName() const;
    int getRoomId() const;
    std::string getSize() const;
    std::string getFlooringType() const;
    Map* getMap() const { return map; }

    // Methods
    void getRoomInfo() const;
    void markClean();   // Mark a room as clean
    void markDirty();   // Mark a room as dirty
    void addNeighbor(Room* neighbor);       // Add a neighbor room
    void setMap(Map* m) { map = m; }

    // Destructor (optional)
    ~Room() = default;
};

#endif // ROOM_H