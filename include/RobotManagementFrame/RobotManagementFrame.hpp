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
#include "Schedular/Schedular.hpp"  // Include Scheduler header

// Forward declarations
class LoginDialog;
class AlertDialog;

class RobotManagementFrame : public wxFrame {
public:
    RobotManagementFrame(const wxString& title = "Robot Management System");
    ~RobotManagementFrame();

private:
    // Methods
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
    void UpdateSchedulerRobotChoices(); // Declare the method



    // Event Handlers
    void OnCheckAlerts(wxTimerEvent& evt);
    void OnRefreshStatus(wxCommandEvent& evt);
    void OnClearAlerts(wxCommandEvent& evt);
    void OnStartCleaning(wxCommandEvent& evt);
    void OnStopCleaning(wxCommandEvent& evt);
    void OnReturnToCharger(wxCommandEvent& evt);
    void OnExit(wxCommandEvent& evt);
    void OnStatusUpdateTimer(wxTimerEvent& evt);
    void OnRefreshAlerts(wxCommandEvent& evt);
    void OnAddRobot(wxCommandEvent& evt);
    void OnDeleteRobot(wxCommandEvent& evt);
    void OnAssignTask(wxCommandEvent& event);


    // Member variables
    std::shared_ptr<MongoDBAdapter> dbAdapter;
    std::unique_ptr<RobotSimulator> simulator_;
    std::unique_ptr<AlertSystem> alertSystem;
    Scheduler scheduler_;
    std::vector<std::shared_ptr<User>> users;
    std::shared_ptr<User> currentUser;
    std::map<std::string, std::string> userPasswords;
    wxListBox* alertsList;
    wxGrid* robotGrid;
    wxTimer* alertCheckTimer;
    wxTimer* statusUpdateTimer;
    static const int ALERT_TIMER_ID = 100;
    static const int STATUS_TIMER_ID = 101;
    wxChoice* strategyChoice;
    wxTextCtrl* roomIdInput;
    MapPanel* mapPanel_;
    wxChoice* robotChoice;
    wxChoice* schedulerRobotChoice; // Add this line


    // Other members
    wxDECLARE_EVENT_TABLE();


    static const std::string DB_URI;
    static const std::string DB_NAME;
    static const std::string MAP_FILE;


};

#endif // ROBOT_MANAGEMENT_FRAME_HPP
