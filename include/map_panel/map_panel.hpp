#ifndef MAP_PANEL_HPP
#define MAP_PANEL_HPP

#include <wx/wx.h>
#include <wx/panel.h>
#include <memory>
#include "RobotSimulator/RobotSimulator.hpp"

class MapPanel : public wxPanel {
public:
    MapPanel(wxWindow* parent, std::shared_ptr<RobotSimulator> simulator);

    void OnPaint(wxPaintEvent& event);
    void OnMouseClick(wxMouseEvent& event);

private:
    std::shared_ptr<RobotSimulator> simulator_;

    wxDECLARE_EVENT_TABLE();
};

#endif // MAP_PANEL_HPP
