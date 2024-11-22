#include <catch2/catch_test_macros.hpp>
#include "map/map.h"
#include "config/ResourceConfig.hpp"
#include "Room/Room.h"
#include "virtual_wall/virtual_wall.h"
#include <memory>
#include <filesystem>

TEST_CASE("Test Mapping System", "[mapping]") {
    // Initialize resource config with the correct path
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path resourcePath = currentPath / "resources";
    if (!std::filesystem::exists(resourcePath)) {
        resourcePath = currentPath / ".." / "resources";
    }
    config::ResourceConfig::initialize(resourcePath.string());
    
    SECTION("Room Class") {
        // Create test rooms
        Room room1("Living Room", 1, "Carpet", false);
        Room room2("Kitchen", 2, "Tile", false);

        // Test room attributes
        REQUIRE(room1.getRoomName() == "Living Room");
        REQUIRE(room1.getRoomId() == 1);
        REQUIRE(room1.flooringType == "Carpet");

        REQUIRE(room2.getRoomName() == "Kitchen");
        REQUIRE(room2.getRoomId() == 2);
        REQUIRE(room2.flooringType == "Tile");
    }

    SECTION("Map Operations") {
        Map map;  // Create a fresh map for testing basic operations
        
        // Test adding rooms
        REQUIRE_NOTHROW(map.addRoom("Living Room", 1, "Carpet", false));
        REQUIRE_NOTHROW(map.addRoom("Kitchen", 2, "Tile", false));

        // Test getting rooms
        auto rooms = map.getRooms();
        REQUIRE(rooms.size() == 2);

        // Test finding room by ID
        auto foundRoom = map.getRoomById(1);
        REQUIRE(foundRoom != nullptr);
        REQUIRE(foundRoom->getRoomName() == "Living Room");

        // Test finding non-existent room
        REQUIRE(map.getRoomById(999) == nullptr);
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
        REQUIRE(chargingStation->flooringType == "Hardwood");
    }

    SECTION("Room Connections") {
        Room room1("Living Room", 1, "Carpet", false);
        Room room2("Kitchen", 2, "Tile", false);
        Room room3("Bedroom", 3, "Carpet", false);

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

    SECTION("Virtual Wall") {
        Room room1("Living Room", 1, "Carpet", false);
        Room room2("Kitchen", 2, "Tile", false);

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
}