#ifndef ROBOT_MANAGEMENT_FRAME_HPP
#define ROBOT_MANAGEMENT_FRAME_HPP

#include "AlertSystem/alert_system.h"
#include "Robot/Robot.h"
#include "adapter/MongoDBAdapter.hpp"
#include "alert/Alert.h"
#include "role/role.h"
#include "user/user.h"
#include "RobotSimulator.hpp"
#include <chrono>
#include <ctime>
#include <map>
#include <memory>
#include <mongocxx/instance.hpp>
#include <thread>
#include <vector>
#include <wx/grid.h>
#include <wx/listbox.h>
#include <wx/notebook.h>
#include <wx/stattext.h>
#include <wx/timer.h>
#include <wx/wx.h>

// Login Dialog with hardcoded credentials
class LoginDialog : public wxDialog {
public:
    LoginDialog(wxWindow* parent);
    std::string GetUsername() const;
    std::string GetPassword() const;

private:
    wxTextCtrl* usernameInput;
    wxTextCtrl* passwordInput;
};

// Alert Dialog for displaying messages
class AlertDialog : public wxDialog {
public:
    AlertDialog(wxWindow* parent, const std::string& message);
};

class RobotManagementFrame : public wxFrame {
public:
    RobotManagementFrame();
    ~RobotManagementFrame();

    void OnRefreshAlerts(wxCommandEvent& evt);

private:
    static mongocxx::instance* mongoInstance;
    std::vector<std::shared_ptr<Robot>> robots;
    std::vector<std::shared_ptr<User>> users;
    std::shared_ptr<User> currentUser;
    std::unique_ptr<AlertSystem> alertSystem;
    std::unique_ptr<MongoDBAdapter> dbAdapter;
    std::unique_ptr<RobotSimulator> simulator_;
    wxGrid* robotGrid;
    wxListBox* alertsList;
    wxTimer* alertTimer;
    std::map<std::string, std::string> userPasswords;

    // Method to load data from MongoDB
    void LoadFromDatabase();
    void InitializeUsers();
    bool ShowLogin();

    // UI creation methods
    void CreateDashboardPanel(wxNotebook* notebook);
    void CreateRobotControlPanel(wxNotebook* notebook);
    void CreateAlertsPanel(wxNotebook* notebook);
    void CreateUserManagementPanel(wxNotebook* notebook);
    void CreateMenuBar();
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
};

#endif // ROBOT_MANAGEMENT_FRAME_HPP
