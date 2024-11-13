#include "map_panel/map_panel.hpp"
#include <cmath>
#include <map>
#include <algorithm>

BEGIN_EVENT_TABLE(MapPanel, wxPanel)
    EVT_PAINT(MapPanel::OnPaint)
END_EVENT_TABLE()

MapPanel::MapPanel(wxWindow* parent, const Map& map, RobotSimulator* simulator)
    : wxPanel(parent), map_(map), simulator_(simulator) {
    // Set background color if needed
    SetBackgroundColour(*wxWHITE);
}

void MapPanel::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);

    // Get the size of the panel
    int width, height;
    GetClientSize(&width, &height);

    // Determine scaling factors
    int margin = 50;
    int drawableWidth = width - 2 * margin;
    int drawableHeight = height - 2 * margin;

    // Get rooms from the map
    const std::vector<Room*>& rooms = map_.getRooms();

    // Map room IDs to positions
    std::map<int, wxPoint> roomPositions;

    // For simplicity, arrange rooms in a circle
    size_t numRooms = rooms.size();
    if (numRooms == 0) return;  // Handle case with no rooms

    double angleIncrement = 2 * M_PI / numRooms;
    double radius = std::min(drawableWidth, drawableHeight) / 2 - margin;

    for (size_t i = 0; i < numRooms; ++i) {
        double angle = i * angleIncrement;
        int x = width / 2 + static_cast<int>(radius * cos(angle));
        int y = height / 2 + static_cast<int>(radius * sin(angle));
        roomPositions[rooms[i]->getRoomId()] = wxPoint(x, y);
    }

    // Draw connections between rooms
    dc.SetPen(*wxBLACK_PEN);
    for (const Room* room : rooms) {
        wxPoint from = roomPositions[room->getRoomId()];
        for (const Room* neighbor : room->neighbors) {
            wxPoint to = roomPositions[neighbor->getRoomId()];
            dc.DrawLine(from, to);
        }
    }

    // Draw virtual walls
    dc.SetPen(wxPen(*wxRED, 2, wxPENSTYLE_DOT));
    for (const VirtualWall& vw : map_.getVirtualWalls()) {
        wxPoint from = roomPositions[vw.getRoom1()->getRoomId()];
        wxPoint to = roomPositions[vw.getRoom2()->getRoomId()];
        dc.DrawLine(from, to);
    }

    // Draw rooms
    int roomRadius = 20;
    for (const Room* room : rooms) {
        wxPoint pos = roomPositions[room->getRoomId()];
        // Set color based on cleanliness
        if (room->isRoomClean) {
            dc.SetBrush(*wxGREEN_BRUSH);
        } else {
            dc.SetBrush(*wxLIGHT_GREY_BRUSH);
        }
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawCircle(pos, roomRadius);
        // Draw room name
        dc.DrawText(room->getRoomName(), pos.x - roomRadius, pos.y - roomRadius - 15);
    }

    // Draw robot positions
    // Get the latest robots from the simulator
    std::vector<std::shared_ptr<Robot>> robots = simulator_->getRobots();

    for (const auto& robot : robots) {
        Room* currentRoom = robot->getCurrentRoom();
        if (currentRoom) {
            wxPoint pos = roomPositions[currentRoom->getRoomId()];
            // Draw robot as a small circle
            dc.SetBrush(*wxBLUE_BRUSH);
            dc.SetPen(*wxBLACK_PEN);
            dc.DrawCircle(pos.x + roomRadius, pos.y + roomRadius, roomRadius / 2);
            // Draw robot name
            dc.DrawText(robot->getName(), pos.x + roomRadius + 10, pos.y + roomRadius - 5);
        }
    }
}
