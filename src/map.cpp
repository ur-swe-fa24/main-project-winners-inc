#include "map/map.h"
#include "Room/Room.h"
#include "virtual_wall/virtual_wall.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <deque>  // Include the deque header
#include <map>
#include <algorithm>
using json = nlohmann::json;

Map::Map() {
    // Constructor implementation
}

Map::~Map() {
    // Destructor implementation
    for (auto room : roomMap) {
        delete room;
    }
}

void Map::addRoom(const std::string& roomName, int id, const std::string& flooringType, bool isRoomClean) {
    Room* newRoom = new Room(roomName, id, flooringType, isRoomClean);
    roomMap.push_back(newRoom);
}

void Map::connectRooms(Room* room1, Room* room2) {
    room1->addNeighbor(room2);
    room2->addNeighbor(room1);
}

void Map::addVirtualWall(Room* room1, Room* room2) {
    VirtualWall newVW(room1, room2);
    virtualWallMap.push_back(newVW);
}

void Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    json j;
    file >> j;

    // Load rooms
    for (const auto& roomData : j["rooms"]) {
        addRoom(roomData["name"], roomData["id"], roomData["flooringType"], roomData["isRoomClean"]);
    }

    // Load connections
    for (const auto& conn : j["connections"]) {
        Room* room1 = getRoomById(conn["room1"]);
        Room* room2 = getRoomById(conn["room2"]);
        if (room1 && room2) {
            connectRooms(room1, room2);
        }
    }

    // Load virtual walls
    for (const auto& vw : j["virtualWalls"]) {
        Room* room1 = getRoomById(vw["room1"]);
        Room* room2 = getRoomById(vw["room2"]);
        if (room1 && room2) {
            addVirtualWall(room1, room2);
        }
    }
}

Room* Map::getRoomById(int id) {
    for (auto room : roomMap) {
        if (room->getRoomId() == id) {
            return room;
        }
    }
    return nullptr;
}

const std::vector<Room*>& Map::getRooms() const {
    return roomMap;
}

const std::vector<VirtualWall>& Map::getVirtualWalls() const {
    return virtualWallMap;
}

std::vector<int> Map::getRoute(Room& start, Room& end){
    // Implementation with BFS
    // Initializing data structures
    std::deque<Room*> queue;  // Use std::deque
    std::vector<int> proposedRoute;
    std::map<int, int> cameFrom; // For reconstructing the path


    // Setting up start
    queue.push_back(&start);
    cameFrom[start.roomId] = -1;

    while(!queue.empty()) {
        Room* current = queue.front();
        queue.pop_front();

        if(current->roomId == end.roomId){
            // Reconstruct the path
            int currentId = end.roomId;
            while(currentId != -1) {
                proposedRoute.push_back(currentId);
                currentId = cameFrom[currentId];
            }
            std::reverse(proposedRoute.begin(), proposedRoute.end());
            return proposedRoute;
        }

        for(Room* neighbor : current->neighbors){
            if(cameFrom.find(neighbor->roomId) == cameFrom.end()){
                queue.push_back(neighbor);
                cameFrom[neighbor->roomId] = current->roomId;
            }
        }
    }
    // Return empty path if no route found
    return proposedRoute;
}
