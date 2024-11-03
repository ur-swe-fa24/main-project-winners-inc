#ifndef AREA_H
#define AREA_H

#include <string>

class Area {
public:
    int id;
    std::string type;     // Floor type?
    bool isAreaClean;     // True when all rooms in area are clean; false otherwise

};

#endif // AREA_H