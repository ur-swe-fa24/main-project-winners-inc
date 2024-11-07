#ifndef VIRTUAL_WALL_H
#define VIRTUAL_WALL_H

#include <string>
#include "room/room.h"

class VirtualWall {
public:
    // Attributes
    int wallID;        // Identifier for an instance of VirtualWall

    // VirtualWall exists between two rooms -- room1 and room2
    Room* room1;
    Room* room2;

    // Constructor
    VirtualWall(int wallID, Room* room1, Room* room2);
};

#endif // VIRTUAL_WALL_H
