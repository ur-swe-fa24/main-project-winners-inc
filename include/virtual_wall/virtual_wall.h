// virtual_wall.h

#ifndef VIRTUAL_WALL_H
#define VIRTUAL_WALL_H

#include "Room/Room.h"
#include <string>
#include "Room/Room.h"


class VirtualWall {
public:
    VirtualWall(Room* room1, Room* room2);
    ~VirtualWall();

    Room* getRoom1() const;
    Room* getRoom2() const;

private:
    Room* room1;
    Room* room2;
};

#endif // VIRTUAL_WALL_H
