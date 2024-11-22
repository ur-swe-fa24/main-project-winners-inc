#include "map_panel/map_panel.hpp"
#include "Room/Room.h"
#include <cmath>
#include <map>
#include <algorithm>
#include <wx/dcbuffer.h> // For double buffering to prevent flickering

BEGIN_EVENT_TABLE(MapPanel, wxPanel)
    EVT_PAINT(MapPanel::OnPaint)
    EVT_LEFT_DOWN(MapPanel::OnMouseClick)  // Add this line to capture left-click events
END_EVENT_TABLE()

MapPanel::MapPanel(wxWindow* parent, const Map& map, RobotSimulator* simulator)
    : wxPanel(parent), map_(map), simulator_(simulator) {
    SetBackgroundColour(*wxWHITE);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void MapPanel::OnPaint(wxPaintEvent& event) {
    if (map_.getRooms().empty()) return;

    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

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
    if (numRooms == 0) return;

    double angleIncrement = 2 * M_PI / numRooms;
    double radius = std::min(drawableWidth, drawableHeight) / 2 - margin;

    // Calculate center of the panel
    int centerX = width / 2;
    int centerY = height / 2;

    // Calculate room positions
    for (size_t i = 0; i < numRooms; ++i) {
        double angle = i * angleIncrement;
        int x = centerX + static_cast<int>(radius * cos(angle));
        int y = centerY + static_cast<int>(radius * sin(angle));
        roomPositions[rooms[i]->getRoomId()] = wxPoint(x, y);
    }

    // Draw connections between rooms
    dc.SetPen(*wxBLACK_PEN);
    for (const Room* room : rooms) {
        if (!room) continue;
        wxPoint from = roomPositions[room->getRoomId()];
        for (const Room* neighbor : room->neighbors) {
            if (!neighbor) continue;
            wxPoint to = roomPositions[neighbor->getRoomId()];
            dc.DrawLine(from, to);
        }
    }

    // Draw virtual walls
    dc.SetPen(wxPen(*wxRED, 2, wxPENSTYLE_DOT));
    const auto& virtualWalls = map_.getVirtualWalls();
    for (const auto& vw : virtualWalls) {
        const Room* room1 = vw.getRoom1();
        const Room* room2 = vw.getRoom2();
        if (!room1 || !room2) continue;
        
        wxPoint from = roomPositions[room1->getRoomId()];
        wxPoint to = roomPositions[room2->getRoomId()];
        dc.DrawLine(from, to);
    }

    // Draw rooms
    int roomRadius = 20;  // Base room radius
    for (const Room* room : rooms) {
        if (!room) continue;
        wxPoint pos = roomPositions[room->getRoomId()];
        
        // Determine circle size based on room size
        int circleRadius = roomRadius;
        if (room->getSize() == "small") {
            circleRadius = roomRadius - 5;
        } else if (room->getSize() == "large") {
            circleRadius = roomRadius + 5;
        }

        // Set color based on room type and cleanliness
        if (room->getRoomId() == 0) {
            dc.SetBrush(*wxYELLOW_BRUSH);  // Charging station
        } else if (room->isRoomClean) {
            dc.SetBrush(*wxGREEN_BRUSH);   // Clean room
        } else {
            dc.SetBrush(*wxLIGHT_GREY_BRUSH);  // Dirty room
        }

        dc.SetPen(*wxBLACK_PEN);
        dc.DrawCircle(pos, circleRadius);

        // Draw room name
        wxString roomLabel = wxString::Format("%s (%d)", 
            wxString::FromUTF8(room->getRoomName()), room->getRoomId());
        dc.DrawText(roomLabel, pos.x - circleRadius, pos.y - circleRadius - 15);
    }

    // Draw robots
    const auto& robots = simulator_->getRobots();
    for (const auto& robot : robots) {
        if (!robot) continue;
        
        Room* currentRoom = robot->getCurrentRoom();
        Room* nextRoom = robot->getNextRoom();
        
        if (!currentRoom) continue;
        
        wxPoint pos;
        int movementProgress = robot->getMovementProgress();

        if (nextRoom && movementProgress > 0) {
            // Interpolate position for moving robots
            wxPoint from = roomPositions[currentRoom->getRoomId()];
            wxPoint to = roomPositions[nextRoom->getRoomId()];
            double progress = 1.0 - static_cast<double>(movementProgress) / 5.0;  // Progress from 0 to 1

            // Calculate position along the path
            pos.x = from.x + static_cast<int>((to.x - from.x) * progress);
            pos.y = from.y + static_cast<int>((to.y - from.y) * progress);
        } else {
            pos = roomPositions[currentRoom->getRoomId()];
        }

        // Draw robot
        dc.SetBrush(*wxBLUE_BRUSH);
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawCircle(pos, roomRadius / 2);  // Robot is half the size of a room
        
        // Draw robot name
        wxString robotName = wxString::FromUTF8(robot->getName());
        dc.DrawText(robotName, pos.x + roomRadius / 2 + 5, pos.y - 5);
    }
}

void MapPanel::OnMouseClick(wxMouseEvent& event) {
    wxPoint pos = event.GetPosition();
    std::cout << "Mouse clicked at: " << pos.x << ", " << pos.y << std::endl;
    event.Skip();
}
