#include "analytics/analytics.h"  // Change .hpp to .h
#include "adapter/MongoDBAdapter.hpp"
#include <tuple>

wxBEGIN_EVENT_TABLE(RobotAnalyticsPanel, wxPanel)
    EVT_BUTTON(wxID_ANY, RobotAnalyticsPanel::OnRefreshClicked)
wxEND_EVENT_TABLE()

RobotAnalyticsPanel::RobotAnalyticsPanel(wxWindow* parent, std::shared_ptr<MongoDBAdapter> dbAdapter)
    : wxPanel(parent), dbAdapter_(dbAdapter) {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    analyticsGrid_ = new wxGrid(this, wxID_ANY);
    analyticsGrid_->CreateGrid(0, 4);
    analyticsGrid_->SetColLabelValue(0, "Robot Name");
    analyticsGrid_->SetColLabelValue(1, "Error Count");
    analyticsGrid_->SetColLabelValue(2, "Total Work Time (s)");
    analyticsGrid_->SetColLabelValue(3, "Error Rate (errors/hr)");

    refreshBtn_ = new wxButton(this, wxID_ANY, "Refresh Analytics");

    sizer->Add(analyticsGrid_, 1, wxEXPAND | wxALL, 5);
    sizer->Add(refreshBtn_, 0, wxALL, 5);

    SetSizer(sizer);
    RefreshAnalytics();
}

void RobotAnalyticsPanel::RefreshAnalytics() {
    if (!dbAdapter_) return;
    auto data = dbAdapter_->retrieveRobotAnalytics();

    // Clear old rows
    int rows = analyticsGrid_->GetNumberRows();
    if (rows > 0) analyticsGrid_->DeleteRows(0, rows);

    for (size_t i = 0; i < data.size(); ++i) {
        analyticsGrid_->AppendRows(1);
        auto& tuple = data[i];
        std::string name; 
        int errorCount; 
        double totalTime;
        std::tie(name, errorCount, totalTime) = tuple;

        double errorRate = 0.0;
        if (totalTime > 0) {
            errorRate = (errorCount / (totalTime / 3600.0)); // errors per hour
        }

        analyticsGrid_->SetCellValue(i, 0, name);
        analyticsGrid_->SetCellValue(i, 1, wxString::Format("%d", errorCount));
        analyticsGrid_->SetCellValue(i, 2, wxString::Format("%.1f", totalTime));
        analyticsGrid_->SetCellValue(i, 3, wxString::Format("%.2f", errorRate));

        // If robot failed (errorCount>0), you could highlight row, etc.
    }

    analyticsGrid_->AutoSize();
}

void RobotAnalyticsPanel::OnRefreshClicked(wxCommandEvent& event) {
    RefreshAnalytics();
}
