#include "map.h"
#include "room.h"
#include "virtual_wall.h"
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>

using namespace std;

// Creating new room object
void addRoom(const std::string& roomName, int id, const std::string& flooringType, bool isRoomClean){
    Room* newRoom = new Room(roomName, id, flooringType, isRoomClean);
}

// Establishes a connection between rooms
void connectRooms(Room* room1, Room* room2){
    room1->addNeighbor(room2);
    room2->addNeighbor(room1);
}

void addVirtualWall(int wallID, Room* room1, Room* room2){
    VirtualWall* newVW = new VirtualWall(wallID, room1, room2)
}

std::vector<int> Map::getRoute(Room& start, Room& end){
    // Implementation with BFS
    // Initializing data structures
    deque<Room*> currentRoom;
    std::vector<int> proposedRoute;

    // Setting up start and adding its roomID to proposedRoute
    currentRoom.push_front(&start);
    proposedRoute.push_back(start.roomId);

    // While currentRoom is not empty, or in other words there are still elements to be visited
    while(!currentRoom.empty())
    {
        Room* current = currentRoom.front();
        currentRoom.pop_back();

        proposedRoute.push_back(current->roomId);               // Push the roomID of the current room to proposedRoute

        // Check if end has been reached
        if(current->roomId == end.roomId){
            return proposedRoute;
        }

        // For neighbor in neighbors...
        for(Room* neighbor : current-> neighbors){
            if(std::find(proposedRoute.begin(), proposedRoute.end(), neighbor->roomId) == proposedRoute.end()){
                //currentRoom.push_back(neighbor);                // Push neighbor to currentRoom so this room becomes current
                Room* currentNeighbor = neighbor;
                proposedRoute.push_back(currentNeighbor->roomID);      // Error - need to fix this
            }
        }
    }
}
