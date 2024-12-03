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
#include "config/ResourceConfig.hpp"

using json = nlohmann::json;

Map::Map(bool loadDefaultMap) {
    if (loadDefaultMap) {
        loadFromFile(config::ResourceConfig::getMapPath());
    }
}

Map::~Map() {
    // Destructor implementation
    for (auto room : roomMap) {
        delete room;
    }
}

void Map::addRoom(const std::string& roomName, int id, const std::string& flooringType, bool isRoomClean) {
    Room* newRoom = new Room(roomName, id, flooringType, "medium", isRoomClean);
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
    std::cout << "Opening map file: " << filename << std::endl;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open map file: " + filename);
    }

    json j;
    try {
        file >> j;
        std::cout << "JSON content loaded successfully" << std::endl;
        
        // Debug print JSON structure
        std::cout << "JSON structure:" << std::endl;
        std::cout << j.dump(2) << std::endl;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("JSON parsing error in map file: " + std::string(e.what()));
    }

    // Load rooms
    std::cout << "Loading rooms..." << std::endl;
    for (const auto& roomData : j["rooms"]) {
        std::string size = roomData.contains("size") ? roomData["size"] : "medium";
        std::cout << "Creating room: " << roomData["name"].get<std::string>() 
                 << ", id: " << roomData["id"].get<int>() 
                 << ", floor: " << roomData["flooringType"].get<std::string>() << std::endl;
        Room* room = new Room(roomData["name"], roomData["id"], roomData["flooringType"], 
                            size, roomData["isRoomClean"]);
        roomMap.push_back(room);
    }

    // Load connections
    std::cout << "Loading connections..." << std::endl;
    for (const auto& conn : j["connections"]) {
        int fromId = conn["from"].get<int>();
        int toId = conn["to"].get<int>();
        std::cout << "Connecting rooms: " << fromId << " -> " << toId << std::endl;
        Room* room1 = getRoomById(fromId);
        Room* room2 = getRoomById(toId);
        if (room1 && room2) {
            connectRooms(room1, room2);
        }
    }

    // Load virtual walls
    std::cout << "Loading virtual walls..." << std::endl;
    if (j.contains("virtualWalls")) {
        for (const auto& vw : j["virtualWalls"]) {
            try {
                int room1Id = vw["room1"].get<int>();
                int room2Id = vw["room2"].get<int>();
                std::cout << "Adding virtual wall between rooms: " << room1Id << " -> " << room2Id << std::endl;
                Room* room1 = getRoomById(room1Id);
                Room* room2 = getRoomById(room2Id);
                if (room1 && room2) {
                    addVirtualWall(room1, room2);
                }
            } catch (const json::exception& e) {
                std::cerr << "Error processing virtual wall: " << e.what() << std::endl;
                std::cerr << "Virtual wall data: " << vw.dump() << std::endl;
                throw;
            }
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

bool Map::isVirtualWallBetween(Room* room1, Room* room2) const {
    for (const auto& vw : virtualWallMap) {
        if ((vw.getRoom1() == room1 && vw.getRoom2() == room2) ||
            (vw.getRoom1() == room2 && vw.getRoom2() == room1)) {
            return true;
        }
    }
    return false;
}


std::vector<int> Map::getRoute(Room& start, Room& end) const {
    // First check if there's a virtual wall between start and end rooms
    if (isVirtualWallBetween(&start, &end)) {
        std::cout << "Map::getRoute: Virtual wall exists between room " << start.getRoomId() 
                 << " and room " << end.getRoomId() << ". No path available." << std::endl;
        return std::vector<int>();
    }

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
            // Skip if there's a virtual wall between current room and neighbor
            if (isVirtualWallBetween(currentRoom, neighbor)) {
                continue;
            }

            // Also skip if there's a virtual wall between neighbor and destination
            if (isVirtualWallBetween(neighbor, &end)) {
                continue;
            }

            int neighbor_id = neighbor->getRoomId();
            int alt = dist[current_id] + 1;
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
        std::cerr << "Map::getRoute: No path found from room " << start.getRoomId() 
                 << " to room " << end.getRoomId() << "." << std::endl;
        return path;
    }
    while (at != start.getRoomId()) {
        path.push_back(at);
        at = prev[at];
    }
    path.push_back(start.getRoomId());
    std::reverse(path.begin(), path.end());

    // Log the computed path
    std::cout << "Map::getRoute: Path from room " << start.getRoomId() << " to room " << end.getRoomId() << ": ";
    for (const auto& roomId : path) {
        std::cout << roomId << " ";
    }
    std::cout << std::endl;

    return path;
}
