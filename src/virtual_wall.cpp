// VirtualWall.cpp

#include "virtual_wall/virtual_wall.h"

// Constructor
VirtualWall::VirtualWall(Room* room1, Room* room2) : room1(room1), room2(room2) {}

// Destructor
VirtualWall::~VirtualWall() {}

// Getter methods
Room* VirtualWall::getRoom1() const {
    return room1;
}

Room* VirtualWall::getRoom2() const {
    return room2;
}
