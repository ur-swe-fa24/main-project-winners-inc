#ifndef VIRTUAL_WALL_H
#define VIRTUAL_WALL_H

#include <vector>
#include "location.h"
using namespace std;

class VirtualWall {
public:
    // Attributes
    int wall_id;
    std::vector<Location> coordinates{};

    // Constructors and Destructors
    VirtualWall() = default;
    ~VirtualWall() = default;

};

#endif // VIRTUAL_WALL_H
