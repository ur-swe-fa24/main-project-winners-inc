#ifndef MAP_PANEL_HPP
#define MAP_PANEL_HPP

#include <wx/wx.h>
#include <wx/panel.h>
#include <vector>
#include <memory>

#include "map/map.h"
#include "Robot/Robot.h"
#include "RobotSimulator/RobotSimulator.hpp"

class MapPanel : public wxPanel {
public:
    MapPanel(wxWindow* parent, const Map& map, RobotSimulator* simulator);

    void OnPaint(wxPaintEvent& event);

private:
    const Map& map_;
    RobotSimulator* simulator_;

    DECLARE_EVENT_TABLE()
};

#endif // MAP_PANEL_HPP
