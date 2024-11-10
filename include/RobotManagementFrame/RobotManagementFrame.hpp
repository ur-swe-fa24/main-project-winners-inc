// RobotManagementFrame.hpp

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
#include "alert_system/alert_system.h"
#include "Robot/Robot.h"
#include "user/user.h"
#include "role/role.h"
#include "alert/Alert.h"
#include "RobotSimulator/RobotSimulator.hpp"

// Forward declarations
class LoginDialog;
class AlertDialog;

class RobotManagementFrame : public wxFrame {
public:
    RobotManagementFrame();
    ~RobotManagementFrame();

private:
    // Methods
    void CreateMenuBar();
    void BindEvents();
    void InitializeUsers();
    bool ShowLogin();
    void LoadFromDatabase();
    void CreateDashboardPanel(wxNotebook* notebook);
    void CreateRobotControlPanel(wxNotebook* notebook);
    void CreateAlertsPanel(wxNotebook* notebook);
    void CreateUserManagementPanel(wxNotebook* notebook);
    void UpdateRobotGrid();
    void CheckAndUpdateAlerts();

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


    // Members
    std::shared_ptr<MongoDBAdapter> dbAdapter;
    std::unique_ptr<RobotSimulator> simulator_;
    std::vector<std::shared_ptr<Robot>> robots;
    std::vector<std::shared_ptr<User>> users;
    std::shared_ptr<User> currentUser;
    std::unique_ptr<AlertSystem> alertSystem;
    wxGrid* robotGrid;
    wxListBox* alertsList;
    wxTimer* alertTimer;
    wxTimer* statusUpdateTimer;
    wxChoice* robotChoice;
    std::map<std::string, std::string> userPasswords;
};

#endif // ROBOT_MANAGEMENT_FRAME_HPP
