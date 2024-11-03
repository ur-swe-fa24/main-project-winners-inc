#ifndef MAP_H
#define MAP_H

#include "area/area.h"
#include "room/room.h"
#include "virtual_wall/virtual_wall.h"
#include "location/location.h"
#include <vector>

class Map {
public:
    // Attributes
    std::vector<Area> areas;
    std::vector<VirtualWall> virtualWalls;

    // Methods
    void getRoute(const Location& start, const Location& end);
    void updateMap();
};

#endif // MAP_H
