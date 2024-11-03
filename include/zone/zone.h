#ifndef ZONE_H
#define ZONE_H

#include "area.h"
#include<string>

class Zone : public Area{
public:
    std::string description;
};

#endif // ZONE_H