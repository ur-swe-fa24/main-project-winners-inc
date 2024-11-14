#include "map/map.h"
#include "Room/Room.h"
#include "virtual_wall/virtual_wall.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <map>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <climits> // For INT_MAX
#include <unordered_set> // Add this line


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
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open map file: " + filename);
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("JSON parsing error in map file: " + std::string(e.what()));
    }

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

Room* Map::getRoomById(int id) const {
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

std::vector<int> Map::getRoute(Room& start, Room& end) const{
    std::unordered_map<int, int> dist;
    std::unordered_map<int, int> prev;
    std::unordered_set<int> visited;
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> pq;

    // Initialize distances
    for (const auto& room : roomMap) {
        dist[room->getRoomId()] = INT_MAX;
    }

    dist[start.getRoomId()] = 0;
    pq.push({0, start.getRoomId()});

    while (!pq.empty()) {
        int current_dist = pq.top().first;
        int current_id = pq.top().second;
        pq.pop();

        if (current_id == end.getRoomId()) {
            break;
        }

        if (visited.find(current_id) != visited.end()) {
            continue;
        }
        visited.insert(current_id);

        Room* currentRoom = getRoomById(current_id);
        if (!currentRoom) {
            continue;
        }

        for (Room* neighbor : currentRoom->neighbors) {
            int neighbor_id = neighbor->getRoomId();
            int alt = dist[current_id] + 1; // Assume cost of 1 per room transition
            if (alt < dist[neighbor_id]) {
                dist[neighbor_id] = alt;
                prev[neighbor_id] = current_id;
                pq.push({alt, neighbor_id});
            }
        }
    }

    // Reconstruct path
    std::vector<int> path;
    int at = end.getRoomId();
    if (prev.find(at) == prev.end() && at != start.getRoomId()) {
        // No path found
        return path;
    }
    while (at != start.getRoomId()) {
        path.push_back(at);
        at = prev[at];
    }
    path.push_back(start.getRoomId());
    std::reverse(path.begin(), path.end());
    return path;
}
