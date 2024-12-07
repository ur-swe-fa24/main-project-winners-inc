#include "map_panel/map_panel.hpp"
#include <wx/dcbuffer.h>
#include "map/map.h"

wxBEGIN_EVENT_TABLE(MapPanel, wxPanel)
    EVT_PAINT(MapPanel::OnPaint)
wxEND_EVENT_TABLE()

MapPanel::MapPanel(wxWindow* parent, std::shared_ptr<RobotSimulator> simulator)
    : wxPanel(parent), simulator_(simulator) {
    SetBackgroundColour(*wxWHITE);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void MapPanel::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    // Example: Just draw robot positions
    auto statuses = simulator_->getRobotStatuses();
    int x = 50; 
    int y = 50;
    for (auto& st : statuses) {
        dc.DrawText(wxString::Format("%s (Battery: %.1f%%)", st.name, st.batteryLevel), x, y);
        y += 20;
    }
}
