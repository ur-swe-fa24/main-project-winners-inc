#include <catch2/catch_test_macros.hpp>
#include "map/map.h"
#include "config/ResourceConfig.hpp"
#include "Room/Room.h"
#include "virtual_wall/virtual_wall.h"
#include <memory>
#include <filesystem>

TEST_CASE("Testing Room"){
    SECTION("Testing Room Constructor and Getters"){
        // Create test rooms
        Room room1("Living Room", 1, "Carpet", "medium", false);
        Room room2("Kitchen", 2, "Tile", "medium", false);

        // Test room attributes
        REQUIRE(room1.getRoomName() == "Living Room");
        REQUIRE(room1.getRoomId() == 1);
        REQUIRE(room1.flooringType == "Carpet");

        REQUIRE(room2.getRoomName() == "Kitchen");
        REQUIRE(room2.getRoomId() == 2);
        REQUIRE(room2.flooringType == "Tile");
    }

    SECTION("Room Connections") {
        Room room1("Living Room", 1, "Carpet", "medium", false);
        Room room2("Kitchen", 2, "Tile", "medium", false);
        Room room3("Bedroom", 3, "Carpet", "medium", false);

        // Connect rooms
        room1.addNeighbor(&room2);
        room2.addNeighbor(&room1);
        room2.addNeighbor(&room3);
        room3.addNeighbor(&room2);

        // Test room connections
        REQUIRE(room1.neighbors.size() == 1);
        REQUIRE(room2.neighbors.size() == 2);
        REQUIRE(room3.neighbors.size() == 1);

        REQUIRE(room1.neighbors[0] == &room2);
        REQUIRE(room2.neighbors[0] == &room1);
        REQUIRE(room2.neighbors[1] == &room3);
        REQUIRE(room3.neighbors[0] == &room2);
    }

    SECTION("Marking Room Clean and Dirty"){
        Room room1("Living Room", 1, "Carpet", "medium", false);
        Room room2("Kitchen", 2, "Tile", "medium", false);
        REQUIRE(room1.isRoomClean == false);
        REQUIRE(room2.isRoomClean == false);

        room1.markClean();
        room2.markClean();
        REQUIRE(room1.isRoomClean == true);
        REQUIRE(room2.isRoomClean == true);

        room1.markDirty();
        room2.markDirty();
        REQUIRE(room1.isRoomClean == false);
        REQUIRE(room2.isRoomClean == false);
    }

    SECTION("Room Add Neighbor") {
        Room room1("Living Room", 1, "Wood", "Large", true);
        Room room2("Kitchen", 2, "Tile", "Medium", true);

        room1.addNeighbor(&room2);

        REQUIRE(room1.neighbors.size() == 1);
        REQUIRE(room1.neighbors[0] == &room2);

        REQUIRE(room2.neighbors.size() == 1);
        REQUIRE(room2.neighbors[0] == &room1);
    }
}

TEST_CASE("Testing Virtual Wall") {
    Room room1("Living Room", 1, "Carpet", "medium", false);
    Room room2("Kitchen", 2, "Tile", "medium", false);

    // Connect rooms
    room1.addNeighbor(&room2);
    room2.addNeighbor(&room1);

    // Create virtual wall
    VirtualWall wall(&room1, &room2);

    // Test virtual wall connections
    REQUIRE(wall.getRoom1() == &room1);
    REQUIRE(wall.getRoom2() == &room2);

    // Test that rooms are still connected (VirtualWall is just a marker)
    REQUIRE(room1.neighbors.size() == 1);
    REQUIRE(room2.neighbors.size() == 1);
    REQUIRE(room1.neighbors[0] == &room2);
    REQUIRE(room2.neighbors[0] == &room1);
}

TEST_CASE("Testing Map"){
    // Initialize resource config with the correct path
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path resourcePath = currentPath / "resources";
    if (!std::filesystem::exists(resourcePath)) {
        resourcePath = currentPath / ".." / "resources";
    }
    config::ResourceConfig::initialize(resourcePath.string());

    SECTION("Map Adds Rooms Correctly") {
        Map map;
        map.addRoom("Living Room", 1, "Wood", "Large", true);
        map.addRoom("Kitchen", 2, "Tile", "Medium", false);
        
        const std::vector<Room*>& rooms = map.getRooms();
        
        //Testing Get Room
        REQUIRE(rooms.size() == 2);
        REQUIRE(rooms[0]->getRoomName() == "Living Room");
        REQUIRE(rooms[1]->getRoomName() == "Kitchen");
        REQUIRE(rooms[0]->getRoomId() == 1);
        REQUIRE(rooms[1]->getRoomId() == 2);

        // Test finding room by ID
        auto foundRoom = map.getRoomById(1);
        REQUIRE(foundRoom != nullptr);
        REQUIRE(foundRoom->getRoomName() == "Living Room");

        // Test finding non-existent room
        REQUIRE(map.getRoomById(999) == nullptr);
    }

    SECTION("Map Connects Rooms Correctly") {
        Map map;
        Room* room1 = new Room("Living Room", 1, "Wood", "Large", true);
        Room* room2 = new Room("Kitchen", 2, "Tile", "Medium", false);
        
        map.connectRooms(room1, room2);

        REQUIRE(room1->neighbors.size() == 1);
        REQUIRE(room1->neighbors[0] == room2);
        REQUIRE(room2->neighbors.size() == 1);
        REQUIRE(room2->neighbors[0] == room1);
    }

    SECTION("Map File Loading") {
        Map map(true);  // Load the default map
        
        // Test that the map was loaded correctly
        auto rooms = map.getRooms();
        REQUIRE(rooms.size() == 11);  // Map has 11 rooms (id 0 to 10)
        
        // Test finding specific rooms from the loaded map
        auto chargingStation = map.getRoomById(0);
        REQUIRE(chargingStation != nullptr);
        REQUIRE(chargingStation->getRoomName() == "Charging Station");
        REQUIRE(chargingStation->flooringType == "tile");
    }

    SECTION("Map Adds Virtual Walls Correctly") {
        Map map;
        Room* room1 = new Room("Living Room", 1, "Wood", "Large", true);
        Room* room2 = new Room("Kitchen", 2, "Tile", "Medium", false);

        map.addVirtualWall(room1, room2);
        
        REQUIRE(map.isVirtualWallBetween(room1, room2));
        REQUIRE(map.isVirtualWallBetween(room2, room1));
    }

    SECTION("Map Adds Virtual Walls Correctly") {
        Map map;
        Room* room1 = new Room("Living Room", 1, "Wood", "Large", true);
        Room* room2 = new Room("Kitchen", 2, "Tile", "Medium", false);

        map.addRoom("Living Room", 1, "Wood", "Large", true);
        map.addRoom("Kitchen", 2, "Tile", "Medium", false);
        map.connectRooms(room1, room2);
        map.addVirtualWall(room1, room2);

        REQUIRE(map.isVirtualWallBetween(room1, room2));
        REQUIRE(map.isVirtualWallBetween(room2, room1));

        std::vector<int> route = map.getRoute(*room1, *room2);
        
        REQUIRE(route.empty());  // No route should exist due to the virtual wall
    }

    SECTION("Map Finds Route Between Rooms") {
        Map map;
        Room* room1 = new Room("Living Room", 1, "Wood", "Large", true);
        Room* room2 = new Room("Kitchen", 2, "Tile", "Medium", false);
        
        map.addRoom("Living Room", 1, "Wood", "Large", true);
        map.addRoom("Kitchen", 2, "Tile", "Medium", false);
        map.connectRooms(room1, room2);
        
        std::vector<int> route = map.getRoute(*room1, *room2);
        
        REQUIRE(route.size() == 2);
        REQUIRE(route[0] == 1);  
        REQUIRE(route[1] == 2); 
    }

    SECTION("Map Finds Complicated Route") {
        Map map;
        Room* room1 = new Room("Living Room", 1, "Wood", "Large", true);
        Room* room2 = new Room("Kitchen", 2, "Tile", "Medium", false);
        Room* room3 = new Room("Bathroom", 3, "Tile", "Small", true);
        
        map.addRoom("Living Room", 1, "Wood", "Large", true);
        map.addRoom("Kitchen", 2, "Tile", "Medium", false);
        map.addRoom("Bathroom", 3, "Tile", "Small", true);
        
        map.connectRooms(room1, room2);
        map.connectRooms(room2, room3);
        
        map.addVirtualWall(room2, room3);

        std::vector<int> route = map.getRoute(*room1, *room3);
        
        REQUIRE(route.size() == 3);
        REQUIRE(route[0] == 1);  // Living Room
        REQUIRE(route[1] == 2);  // Kitchen
        REQUIRE(route[2] == 3);  // Bathroom
    }
}

TEST_CASE("Map Loads Correctly From File") {
    // Initialize resource config with the correct path
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path resourcePath = currentPath / "resources";
    if (!std::filesystem::exists(resourcePath)) {
        resourcePath = currentPath / ".." / "resources";
    }
    config::ResourceConfig::initialize(resourcePath.string());
    
    Map map(true);  // Load the default map
    
    // Test that the map was loaded correctly
    auto rooms = map.getRooms();
    REQUIRE(rooms.size() == 11);  // Map has 11 rooms (id 0 to 10)
    
    // Test finding specific rooms from the loaded map
    auto chargingStation = map.getRoomById(0);
    REQUIRE(chargingStation != nullptr);
    REQUIRE(chargingStation->getRoomName() == "Charging Station");
    REQUIRE(chargingStation->flooringType == "tile");
}
