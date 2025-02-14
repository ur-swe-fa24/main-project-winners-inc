#ifndef ROBOT_MANAGEMENT_FRAME_HPP
#define ROBOT_MANAGEMENT_FRAME_HPP

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/grid.h>
#include <wx/timer.h>
#include <memory>
#include <vector>
#include <map>
#include "adapter/MongoDBAdapter.hpp"
#include "AlertSystem/alert_system.h"
#include "Robot/Robot.h"
#include "user/user.h"
#include "role/role.h"
#include "alert/Alert.h"
#include "RobotSimulator/RobotSimulator.hpp"
#include "map_panel/map_panel.hpp"
#include "Scheduler/Scheduler.hpp"
#include "robot_control/robot_control_panel.hpp"
#include "scheduler_panel/scheduler_panel.hpp"

class LoginDialog;
class AlertDialog;

enum {
    ALERT_TIMER_ID = wxID_HIGHEST + 1,
    STATUS_TIMER_ID
};

class RobotManagementFrame : public wxFrame {
public:
    RobotManagementFrame(const wxString& title = "Robot Management System");
    virtual ~RobotManagementFrame();

    void AddAlert(const Alert& alert);  // Method to add alerts

private:
    void InitializeUsers();
    void CreateMenuBar();
    void BindEvents();
    bool ShowLogin();
    void LoadFromDatabase();
    void CreateDashboardPanel(wxNotebook* notebook);
    void CreateRobotControlPanel(wxNotebook* notebook);
    void CreateAlertsPanel(wxNotebook* notebook);
    void CreateUserManagementPanel(wxNotebook* notebook);
    void CreateRobotAnalyticsPanel(wxNotebook* notebook);
    void UpdateRobotGrid();
    void CheckAndUpdateAlerts();
    void CreateMapPanel(wxNotebook* notebook);
    void CreateSchedulerPanel(wxNotebook* notebook);
    void UpdateRobotChoices();
    void UpdateSchedulerRobotChoices();
    void UpdateCleaningStrategies();

    void OnCheckAlerts(wxTimerEvent& evt);
    void OnRefreshStatus(wxCommandEvent& evt);
    void OnClearAlerts(wxCommandEvent& evt);
    void OnRefreshAlerts(wxCommandEvent& evt);
    void OnStartCleaning(wxCommandEvent& evt);
    void OnStopCleaning(wxCommandEvent& evt);
    void OnReturnToCharger(wxCommandEvent& evt);
    void OnStatusUpdateTimer(wxTimerEvent& evt);
    void OnExit(wxCommandEvent& evt);
    void OnAssignTask(wxCommandEvent& event);
    void OnRoomSelected(wxCommandEvent& event);

    static const std::string DB_URI;
    static const std::string DB_NAME;
    std::shared_ptr<MongoDBAdapter> dbAdapter;
    std::shared_ptr<AlertSystem> alertSystem;    // Changed to shared_ptr
    std::shared_ptr<RobotSimulator> simulator_;  // Changed to shared_ptr
    std::shared_ptr<Scheduler> scheduler_;       // Changed to shared_ptr
    std::shared_ptr<User> currentUser;

    wxGrid* robotGrid;
    wxTimer* statusUpdateTimer;
    wxTimer* alertCheckTimer;
    MapPanel* mapPanel_;  
    RobotControlPanel* robotControlPanel;
    SchedulerPanel* schedulerPanel_;
    wxListBox* alertsList;
    wxTextCtrl* roomIdInput;

    std::vector<std::shared_ptr<User>> users;
    std::map<std::string, std::string> userPasswords;

    DECLARE_EVENT_TABLE()
};

#endif // ROBOT_MANAGEMENT_FRAME_HPP
