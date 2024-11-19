#define CATCH_CONFIG_MAIN
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "map/map.h"
#include "Room/Room.h"
#include "virtual_wall/virtual_wall.h"

TEST_CASE("Room functionality") {
    // Create Room objects
    Room room1("Living Room", 1, "Carpet", true);
    Room room2("Kitchen", 2, "Tile", false);

    SECTION("Room getters work correctly") {
        REQUIRE(room1.getRoomName() == "Living Room");
        REQUIRE(room1.getRoomId() == 1);
        REQUIRE(room1.isRoomClean == true);

        REQUIRE(room2.getRoomName() == "Kitchen");
        REQUIRE(room2.getRoomId() == 2);
        REQUIRE(room2.isRoomClean == false);
    }

    SECTION("Room neighbors functionality") {
        room1.addNeighbor(&room2);
        REQUIRE(room1.neighbors.size() == 1);
        REQUIRE(room1.neighbors[0]->getRoomId() == 2);

        room2.addNeighbor(&room1);
        REQUIRE(room2.neighbors.size() == 1);
        REQUIRE(room2.neighbors[0]->getRoomId() == 1);
    }

    SECTION("Room cleaning status updates") {
        room2.markClean();
        REQUIRE(room2.isRoomClean == true);

        room1.markDirty();
        REQUIRE(room1.isRoomClean == false);
    }
}

TEST_CASE("VirtualWall functionality"){
    Room room1("Living Room", 1, "Carpet", true);
    Room room2("Kitchen", 2, "Tile", false);

    // Create a VirtualWall instance
    VirtualWall wall(&room1, &room2);

    // Test the constructor and getter methods
    SECTION("Constructor assigns rooms correctly") {
        REQUIRE(wall.getRoom1() == &room1);
        REQUIRE(wall.getRoom2() == &room2);
    }

    SECTION("Rooms are not the same") {
        REQUIRE(wall.getRoom1() != wall.getRoom2());
    }
}

TEST_CASE("Map functionality") {
    // Create Map object
    Map map;

    // Create Room objects
    Room room1("Living Room", 1, "Carpet", true);
    Room room2("Kitchen", 2, "Tile", false);
    Room room3("Bedroom", 3, "Wood", true);

    // Add rooms to the map
    map.addRoom("Living Room", 1, "Carpet", true);
    map.addRoom("Kitchen", 2, "Tile", false);
    map.addRoom("Bedroom", 3, "Wood", true);

    SECTION("Map connection between rooms") {
        map.connectRooms(&room1, &room2);
        REQUIRE(room1.neighbors.size() == 1);
        REQUIRE(room1.neighbors[0]->getRoomId() == 2);

        REQUIRE(room2.neighbors.size() == 1);
        REQUIRE(room2.neighbors[0]->getRoomId() == 1);
    }

    SECTION("VirtualWall addition") {
        map.addVirtualWall(&room1, &room2);
        // Assuming virtual walls are stored in a container
        REQUIRE(map.isVirtualWallBetween(&room1, &room2) == true);
    }

    SECTION("Route finding with BFS") {
        // Connect rooms in a path
        map.connectRooms(&room1, &room2);
        map.connectRooms(&room2, &room3);

        std::vector<int> route = map.getRoute(room1, room3);
        REQUIRE(route == std::vector<int>{1, 2, 3});
    }

    SECTION("Route not found") {
        Room isolatedRoom("Garage", 4, "Concrete", false);
        std::vector<int> route = map.getRoute(room1, isolatedRoom);
        REQUIRE(route.empty());
    }
}