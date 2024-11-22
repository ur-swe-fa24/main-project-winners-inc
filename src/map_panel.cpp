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
    // Set background color if needed
    SetBackgroundColour(*wxWHITE);

    // Set the background style to support double buffering
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void MapPanel::OnPaint(wxPaintEvent& event) {
    if (map_.getRooms().empty()) return;  // Safeguard against empty data

    wxAutoBufferedPaintDC dc(this);  // Switch to buffered DC to prevent flickering and crashes
    dc.Clear(); // Clear the background

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
        if (room->getRoomId() == 0) {
            dc.SetBrush(*wxYELLOW_BRUSH); // Charging station color
        } else if (room->isRoomClean) {
            dc.SetBrush(*wxGREEN_BRUSH);
        } else {
            dc.SetBrush(*wxLIGHT_GREY_BRUSH);
        }
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawCircle(pos, roomRadius);
        // Draw room name
        // Updated code
        wxString roomLabel = wxString::Format("%s (%d)", room->getRoomName(), room->getRoomId());
        dc.DrawText(roomLabel, pos.x - roomRadius, pos.y - roomRadius - 15);
    }

    // Draw robot positions
    // Get the latest robots from the simulator
    std::vector<std::shared_ptr<Robot>> robots = simulator_->getRobots();

    for (const auto& robot : robots) {
        Room* currentRoom = robot->getCurrentRoom();
        Room* nextRoom = robot->getNextRoom();
        int movementProgress = robot->getMovementProgress();
        wxPoint pos;

        if (nextRoom && movementProgress > 0) {
            // Interpolate position
            wxPoint from = roomPositions[currentRoom->getRoomId()];
            wxPoint to = roomPositions[nextRoom->getRoomId()];
            double progress = 1.0 - static_cast<double>(movementProgress) / 5.0; // Divisor matches movementProgress_ maximum value

            pos.x = from.x + static_cast<int>((to.x - from.x) * progress);
            pos.y = from.y + static_cast<int>((to.y - from.y) * progress);

        } else if (currentRoom) {
            // Robot is stationary
            pos = roomPositions[currentRoom->getRoomId()];
        } else {
            // Default position
            pos = wxPoint(width / 2, height / 2);
        }

        // Draw robot as a small circle
        dc.SetBrush(*wxBLUE_BRUSH);
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawCircle(pos, roomRadius / 2);
        // Draw robot name
        dc.DrawText(robot->getName(), pos.x + roomRadius / 2 + 5, pos.y - 5);
    }
}

void MapPanel::OnMouseClick(wxMouseEvent& event) {
    // Handle mouse click here
    wxPoint pos = event.GetPosition();
    std::cout << "Mouse clicked at: " << pos.x << ", " << pos.y << std::endl;

    // Add logic here if you want to interact with specific areas of the map

    event.Skip();  // Allow the event to propagate if not fully handled
}
