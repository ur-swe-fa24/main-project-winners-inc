#include "map_panel/map_panel.hpp"
#include <wx/dcbuffer.h>
#include <cmath>
#include <map>
#include <algorithm>
#include <iostream>
#include "map/map.h"
#include "Room/Room.h"
#include "Robot/Robot.h" // Include if needed for robot access

wxBEGIN_EVENT_TABLE(MapPanel, wxPanel)
    EVT_PAINT(MapPanel::OnPaint)
    EVT_LEFT_DOWN(MapPanel::OnMouseClick) // Capture left-click events
wxEND_EVENT_TABLE()

MapPanel::MapPanel(wxWindow* parent, std::shared_ptr<RobotSimulator> simulator)
    : wxPanel(parent), simulator_(simulator) {
    SetBackgroundColour(*wxWHITE);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void MapPanel::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    const Map& map = simulator_->getMap();
    const std::vector<Room*>& rooms = map.getRooms();
    if (rooms.empty()) return;

    // Get panel size
    int width, height;
    GetClientSize(&width, &height);

    // Determine scaling factors and layout
    int margin = 50;
    int drawableWidth = width - 2 * margin;
    int drawableHeight = height - 2 * margin;
    size_t numRooms = rooms.size();
    if (numRooms == 0) return;

    double angleIncrement = 2 * M_PI / numRooms;
    double radius = std::min(drawableWidth, drawableHeight) / 2 - margin;
    int centerX = width / 2;
    int centerY = height / 2;

    // Map room IDs to positions
    std::map<int, wxPoint> roomPositions;
    for (size_t i = 0; i < numRooms; ++i) {
        double angle = i * angleIncrement;
        int x = centerX + static_cast<int>(radius * std::cos(angle));
        int y = centerY + static_cast<int>(radius * std::sin(angle));
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
    const auto& virtualWalls = map.getVirtualWalls();
    for (const auto& vw : virtualWalls) {
        const Room* room1 = vw.getRoom1();
        const Room* room2 = vw.getRoom2();
        if (!room1 || !room2) continue;
        
        wxPoint from = roomPositions[room1->getRoomId()];
        wxPoint to = roomPositions[room2->getRoomId()];
        dc.DrawLine(from, to);
    }

    // Draw rooms
    int roomRadius = 20;
    for (const Room* room : rooms) {
        if (!room) continue;
        wxPoint pos = roomPositions[room->getRoomId()];
        
        // Adjust radius based on room size
        int circleRadius = roomRadius;
        if (room->getSize() == "small") {
            circleRadius = roomRadius - 5;
        } else if (room->getSize() == "large") {
            circleRadius = roomRadius + 5;
        }

        // Set brush based on cleanliness and whether it's a charger
        if (room->getRoomId() == 0) {
            dc.SetBrush(*wxYELLOW_BRUSH); // Charging station
        } else if (room->isRoomClean) {
            dc.SetBrush(*wxGREEN_BRUSH); // Clean room
        } else {
            dc.SetBrush(*wxLIGHT_GREY_BRUSH); // Dirty room
        }

        dc.SetPen(*wxBLACK_PEN);
        dc.DrawCircle(pos, circleRadius);

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
        double movementProgress = robot->getMovementProgress(); // Ensure double or convert

        if (nextRoom && movementProgress > 0.0) {
            wxPoint from = roomPositions[currentRoom->getRoomId()];
            wxPoint to = roomPositions[nextRoom->getRoomId()];

            // movementProgress should be between 0 and 100, we normalize to [0,1]
            double progress = movementProgress / 100.0;
            // If your old logic used /5.0, ensure movementProgress scale is consistent.
            // Adjust as per your logic. For now, assume movementProgress in %:
            pos.x = from.x + static_cast<int>((to.x - from.x) * progress);
            pos.y = from.y + static_cast<int>((to.y - from.y) * progress);
        } else {
            pos = roomPositions[currentRoom->getRoomId()];
        }

        dc.SetBrush(*wxBLUE_BRUSH);
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawCircle(pos, roomRadius / 2);

        wxString robotName = wxString::FromUTF8(robot->getName());
        dc.DrawText(robotName, pos.x + roomRadius / 2 + 5, pos.y - 5);
    }
}

void MapPanel::OnMouseClick(wxMouseEvent& event) {
    wxPoint pos = event.GetPosition();
    std::cout << "Mouse clicked at: " << pos.x << ", " << pos.y << std::endl;
    event.Skip();
}
