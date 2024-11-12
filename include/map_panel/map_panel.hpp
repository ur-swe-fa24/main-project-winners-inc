// MapPanel.hpp
#ifndef MAP_PANEL_HPP
#define MAP_PANEL_HPP

#include <wx/wx.h>
#include <wx/panel.h>
#include "map/map.h"
#include "Robot/Robot.h"

class MapPanel : public wxPanel {
public:
    MapPanel(wxWindow* parent, const Map& map, const std::vector<std::shared_ptr<Robot>>& robots);

private:
    void OnPaint(wxPaintEvent& event);
    const Map& map_;
    const std::vector<std::shared_ptr<Robot>>& robots_;

    DECLARE_EVENT_TABLE()
};

#endif // MAP_PANEL_HPP
