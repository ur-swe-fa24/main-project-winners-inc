#ifndef MAP_H
#define MAP_H

#include "Room/Room.h"
#include "virtual_wall.h"
#include <string>
#include <vector>

class Map {
  private:
    // Data structures for keeping track of rooms (as well as the connections between them) and virtual walls
    // Subject to change for the time being
    std::vector<Room *> roomMap;
    std::vector<VirtualWall> virtualWallMap;

  public:
    // Constructor and destructor
    Map();
    ~Map();

    // Setting up or making changes to the map
    void addRoom(int id, const std::string &flooringType, bool isRoomClean);
    void connectRooms(Room *room1, Room *room2);   // Could also achieve this by passing room ID numbers
    void addVirtualWall(Room *room1, Room *room2); // Could also achieve this by passing room ID numbers

    // Find some route from one room to another; returns a vector of room ID numbers in the order they should be visited
    std::vector<int> getRoute(Room &start, Room &end);
};

#endif // MAP_H
