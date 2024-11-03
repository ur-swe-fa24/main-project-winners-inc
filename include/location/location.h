#ifndef LOCATION_H
#define LOCATION_H

#include <string>

struct Location{
    std::string room;
    std::string zone;

    Location(const std::string& roomName = "", const std::string& zoneName = "")
        : room(roomName), zone(zoneName) {}
};

#endif // LOCATION_H