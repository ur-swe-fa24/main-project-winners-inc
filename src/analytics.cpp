#include "analytics/analytics.h"
#include "adapter/MongoDBAdapter.hpp"
#include "RobotSimulator/RobotSimulator.hpp"
#include "Robot/Robot.h"
#include <tuple>

wxBEGIN_EVENT_TABLE(RobotAnalyticsPanel, wxPanel)
    EVT_BUTTON(wxID_ANY, RobotAnalyticsPanel::OnRefreshClicked)
wxEND_EVENT_TABLE()

RobotAnalyticsPanel::RobotAnalyticsPanel(wxWindow* parent, std::shared_ptr<MongoDBAdapter> dbAdapter, std::shared_ptr<RobotSimulator> simulator)
    : wxPanel(parent), dbAdapter_(dbAdapter), simulator_(simulator) {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    analyticsGrid_ = new wxGrid(this, wxID_ANY);
    analyticsGrid_->CreateGrid(0, 4);
    analyticsGrid_->SetColLabelValue(0, "Robot Name");
    analyticsGrid_->SetColLabelValue(1, "Error Count");
    analyticsGrid_->SetColLabelValue(2, "Total Work Time (s)");
    analyticsGrid_->SetColLabelValue(3, "Error Rate (errors/hour)");

    refreshBtn_ = new wxButton(this, wxID_ANY, "Refresh Analytics");

    sizer->Add(analyticsGrid_, 1, wxEXPAND | wxALL, 5);
    sizer->Add(refreshBtn_, 0, wxALL, 5);

    SetSizer(sizer);
    RefreshAnalytics();
}

void RobotAnalyticsPanel::RefreshAnalytics() {
    if (!dbAdapter_ || !simulator_) return;
    auto data = dbAdapter_->retrieveRobotAnalytics(); 
    // data: vector<tuple<string, int, double>> -> (name, errorCount, totalTime)

    // Create a map for quick lookup
    std::map<std::string, std::tuple<int,double>> analyticsMap;
    for (auto &entry : data) {
        std::string name;
        int errorCount;
        double totalTime;
        std::tie(name, errorCount, totalTime) = entry;
        analyticsMap[name] = std::make_tuple(errorCount, totalTime);
    }

    // Clear old rows
    int rows = analyticsGrid_->GetNumberRows();
    if (rows > 0) analyticsGrid_->DeleteRows(0, rows);

    // Get the list of robots from the simulator
    const auto& robots = simulator_->getRobots();

    for (size_t i = 0; i < robots.size(); ++i) {
        const auto& robot = robots[i];
        if (!robot) continue;

        std::string name = robot->getName();
        int errorCount = 0;
        double totalTime = 0.0;
        
        // If we have data in the map for this robot
        if (analyticsMap.find(name) != analyticsMap.end()) {
            std::tie(errorCount, totalTime) = analyticsMap[name];
        }

        // Calculate error rate (errors per hour)
        double errorRate = 0.0;
        if (totalTime > 0) {
            errorRate = errorCount / (totalTime / 3600.0);
        }

        analyticsGrid_->AppendRows(1);
        int row = (int)analyticsGrid_->GetNumberRows() - 1;
        analyticsGrid_->SetCellValue(row, 0, name);
        analyticsGrid_->SetCellValue(row, 1, wxString::Format("%d", errorCount));
        analyticsGrid_->SetCellValue(row, 2, wxString::Format("%.1f", totalTime));
        analyticsGrid_->SetCellValue(row, 3, wxString::Format("%.2f", errorRate));
    }

    analyticsGrid_->AutoSize();
}

void RobotAnalyticsPanel::OnRefreshClicked(wxCommandEvent& event) {
    RefreshAnalytics();
}
