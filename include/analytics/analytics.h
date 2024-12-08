#ifndef ROBOT_ANALYTICS_PANEL_HPP
#define ROBOT_ANALYTICS_PANEL_HPP

#include <wx/wx.h>
#include <wx/grid.h>
#include <memory>
#include <vector>
#include <string>

class MongoDBAdapter;
class RobotSimulator;

class RobotAnalyticsPanel : public wxPanel {
public:
    RobotAnalyticsPanel(wxWindow* parent, std::shared_ptr<MongoDBAdapter> dbAdapter, std::shared_ptr<RobotSimulator> simulator);
    ~RobotAnalyticsPanel() {}

private:
    void RefreshAnalytics();
    void OnRefreshClicked(wxCommandEvent& event);

    wxGrid* analyticsGrid_;
    wxButton* refreshBtn_;
    std::shared_ptr<MongoDBAdapter> dbAdapter_;
    std::shared_ptr<RobotSimulator> simulator_;

    wxDECLARE_EVENT_TABLE();
};

#endif
