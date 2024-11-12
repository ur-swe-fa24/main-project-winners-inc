#ifndef MAP_H
#define MAP_H

#include <vector>
#include <string>
#include "Room/Room.h"
#include "virtual_wall/virtual_wall.h"

class Map {
private:
    std::vector<Room*> roomMap;
    std::vector<VirtualWall> virtualWallMap;


public:
    // Constructor and destructor
    Map();
    ~Map();

    // Setting up or making changes to the map
    void addRoom(const std::string& roomName, int id, const std::string& flooringType, bool isRoomClean);
    void connectRooms(Room* room1, Room* room2);
    void addVirtualWall(Room* room1, Room* room2);
    void loadFromFile(const std::string& filename);
    Room* getRoomById(int id);
    const std::vector<Room*>& getRooms() const;
    const std::vector<VirtualWall>& getVirtualWalls() const;

    // Find some route from one room to another; returns a vector of room ID numbers in the order they should be visited
    std::vector<int> getRoute(Room& start, Room& end);
};

#endif // MAP_H
