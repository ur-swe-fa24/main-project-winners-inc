#ifndef ROBOT_ANALYTICS_PANEL_HPP
#define ROBOT_ANALYTICS_PANEL_HPP

#include <wx/wx.h>
#include <wx/grid.h>
#include <memory>

class MongoDBAdapter;

class RobotAnalyticsPanel : public wxPanel {
public:
    RobotAnalyticsPanel(wxWindow* parent, std::shared_ptr<MongoDBAdapter> dbAdapter);

    void RefreshAnalytics();

private:
    void OnRefreshClicked(wxCommandEvent& event);

    std::shared_ptr<MongoDBAdapter> dbAdapter_;
    wxGrid* analyticsGrid_;
    wxButton* refreshBtn_;

    wxDECLARE_EVENT_TABLE();
};

#endif // ROBOT_ANALYTICS_PANEL_HPP
