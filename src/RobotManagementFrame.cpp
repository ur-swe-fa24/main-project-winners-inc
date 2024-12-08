#include "RobotManagementFrame/RobotManagementFrame.hpp"
#include "LoginDialog/LoginDialog.hpp"
#include "AlertDialog/AlertDialog.hpp"
#include "map_panel/map_panel.hpp"
#include "config/ResourceConfig.hpp"
#include <wx/notebook.h>
#include <wx/grid.h>
#include <iostream>
#include <wx/filename.h>
#include "AlertSystem/alert_system.h"
#include "RobotSimulator/RobotSimulator.hpp"
#include "map/map.h"
#include "Scheduler/Scheduler.hpp"
#include "robot_control/robot_control_panel.hpp"
#include "scheduler_panel/scheduler_panel.hpp"
#include "user/user.h"
#include "role/role.h"
#include "permission/permission.h"
#include "alert/Alert.h"
#include "adapter/MongoDBAdapter.hpp"

const std::string RobotManagementFrame::DB_URI = "mongodb://localhost:27017";
const std::string RobotManagementFrame::DB_NAME = "mydb9";

RobotManagementFrame::RobotManagementFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1024, 768)),
      robotControlPanel(nullptr),
      schedulerPanel_(nullptr),
      mapPanel_(nullptr),
      statusUpdateTimer(nullptr),
      alertCheckTimer(nullptr)
{
    try {
        CreateMenuBar();
        CreateStatusBar(2);
        SetStatusText("Initializing...");

        config::ResourceConfig::initialize();

        wxString cwd = wxGetCwd();
        std::cout << "Current working directory: " << cwd.mb_str() << std::endl;

        // Initialize DB
        try {
            dbAdapter = std::make_shared<MongoDBAdapter>(DB_URI, DB_NAME);
            SetStatusText("Connected to database");
        } catch (const std::exception& e) {
            wxMessageBox(wxString::Format("Failed to connect to database: %s", e.what()),
                "Database Error", wxOK | wxICON_ERROR);
            Close(true);
            return;
        }

        // Create map
        auto map = std::make_shared<Map>(true); 
        dbAdapter->initializeRooms(map->getRooms());

        // Create alert system
        alertSystem = std::make_shared<AlertSystem>();

        // Create simulator without scheduler initially
        simulator_ = std::make_shared<RobotSimulator>(map, nullptr, alertSystem, dbAdapter);

        simulator_->addRobot("RobotA");
        simulator_->addRobot("RobotB");
        simulator_->addRobot("RobotC");
        SetStatusText("Simulator initialized");

            // Create scheduler after we have robots in simulator
        scheduler_ = std::make_shared<Scheduler>(map.get(), &simulator_->getRobots());
        // After creating the scheduler
        scheduler_->setSimulator(simulator_);
        scheduler_->setAlertSystem(alertSystem);
        scheduler_->setDbAdapter(dbAdapter);

        // IMPORTANT: Set the simulator in the scheduler
        scheduler_->setSimulator(simulator_);
        InitializeUsers();
        if (!ShowLogin()) {
            Close(true);
            return;
        }

        wxNotebook* notebook = new wxNotebook(this, wxID_ANY);
        SetSizer(new wxBoxSizer(wxVERTICAL));
        GetSizer()->Add(notebook, 1, wxEXPAND);

        if (currentUser->getRole()->hasPermission("Dashboard")) {
            CreateDashboardPanel(notebook);
        }

        if (currentUser->getRole()->hasPermission("Scheduler")) {
            // Pass alertSystem and dbAdapter as well
            schedulerPanel_ = new SchedulerPanel(notebook, simulator_, scheduler_, alertSystem, dbAdapter);
            notebook->AddPage(schedulerPanel_, "Scheduler");
        }

        if (currentUser->getRole()->hasPermission("Alert")) {
            CreateAlertsPanel(notebook);
        }

        if (currentUser->getRole()->hasPermission("Map")) {
            mapPanel_ = new MapPanel(notebook, simulator_);
            notebook->AddPage(mapPanel_, "Map");
        }

        if (currentUser->getRole()->hasPermission("Robot Control")) {
            robotControlPanel = new RobotControlPanel(notebook, simulator_, scheduler_);
            notebook->AddPage(robotControlPanel, "Robot Control");
        }

        if (currentUser->getRole()->hasPermission("Robot Analytics")) {
            CreateRobotAnalyticsPanel(notebook);
        }

        if (currentUser && currentUser->getRole()->hasPermission("ADMIN")) {
            CreateUserManagementPanel(notebook);
        }

        alertCheckTimer = new wxTimer(this, ALERT_TIMER_ID);
        alertCheckTimer->Start(5000); 

        statusUpdateTimer = new wxTimer(this, STATUS_TIMER_ID);
        statusUpdateTimer->Start(1000);

        BindEvents();

        SetStatusText("Ready");
        SetStatusText(wxString::Format("Logged in as: %s (%s)",
            currentUser->getName(), currentUser->getRole()->getName()), 1);

        UpdateRobotChoices();
        UpdateSchedulerRobotChoices();
        UpdateRobotGrid();

        Center();
    } catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Initialization failed: %s", e.what()), "Error",
                     wxOK | wxICON_ERROR);
        Close();
        return;
    }
}


RobotManagementFrame::~RobotManagementFrame() {
    if (statusUpdateTimer) {
        statusUpdateTimer->Stop();
        delete statusUpdateTimer;
    }
    if (alertCheckTimer) {
        alertCheckTimer->Stop();
        delete alertCheckTimer;
    }

    robotControlPanel = nullptr;
    schedulerPanel_ = nullptr;
    mapPanel_ = nullptr;
}


void RobotManagementFrame::CreateMenuBar() {
    wxMenuBar* menuBar = new wxMenuBar();
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_EXIT, "Exit\tAlt-X");
    menuBar->Append(fileMenu, "File");
    SetMenuBar(menuBar);
    Bind(wxEVT_MENU, &RobotManagementFrame::OnExit, this, wxID_EXIT);
}

void RobotManagementFrame::OnCheckAlerts(wxTimerEvent& evt) {
    CheckAndUpdateAlerts();
}

void RobotManagementFrame::InitializeUsers() {
    auto buildingManagerRole = std::make_shared<Role>("Building Manager");
    buildingManagerRole->addPermission(Permission("Dashboard"));
    buildingManagerRole->addPermission(Permission("Scheduler"));
    buildingManagerRole->addPermission(Permission("Alert"));
    buildingManagerRole->addPermission(Permission("Map"));

    auto fieldEngineerRole = std::make_shared<Role>("Field Engineer");
    fieldEngineerRole->addPermission(Permission("Dashboard"));
    fieldEngineerRole->addPermission(Permission("Scheduler"));
    fieldEngineerRole->addPermission(Permission("Alert"));
    fieldEngineerRole->addPermission(Permission("Map"));
    fieldEngineerRole->addPermission(Permission("Robot Control"));

    auto seniorManagerRole = std::make_shared<Role>("Senior Manager");
    seniorManagerRole->addPermission(Permission("Dashboard"));
    seniorManagerRole->addPermission(Permission("Scheduler"));
    seniorManagerRole->addPermission(Permission("Alert"));
    seniorManagerRole->addPermission(Permission("Map"));
    seniorManagerRole->addPermission(Permission("Robot Control"));
    seniorManagerRole->addPermission(Permission("Robot Analytics"));

    users.push_back(std::make_shared<User>("building_manager", buildingManagerRole));
    users.push_back(std::make_shared<User>("field_engineer", fieldEngineerRole));
    users.push_back(std::make_shared<User>("senior_manager", seniorManagerRole));

    userPasswords["building_manager"] = "bm123";
    userPasswords["field_engineer"] = "fe123";
    userPasswords["senior_manager"] = "sm123";
}

void RobotManagementFrame::LoadFromDatabase() {
    auto alerts = dbAdapter->retrieveAlerts();
    for (const auto& alert : alerts) {
        alertsList->Append(alert.getMessage());
    }
    UpdateRobotGrid();
}

void RobotManagementFrame::CreateAlertsPanel(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    alertsList = new wxListBox(panel, wxID_ANY);
    sizer->Add(alertsList, 1, wxEXPAND | wxALL, 5);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* clearBtn = new wxButton(panel, wxID_ANY, "Clear Alerts");
    clearBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnClearAlerts, this);
    buttonSizer->Add(clearBtn, 1, wxALL, 5);

    wxButton* refreshBtn = new wxButton(panel, wxID_ANY, "Refresh Alerts");
    refreshBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnRefreshAlerts, this);
    buttonSizer->Add(refreshBtn, 1, wxALL, 5);

    sizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
    panel->SetSizer(sizer);
    notebook->AddPage(panel, "Alerts");
}

void RobotManagementFrame::CreateRobotControlPanel(wxNotebook* notebook) {
    // Already created inline after login checks in the constructor block.
}

void RobotManagementFrame::CreateDashboardPanel(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    robotGrid = new wxGrid(panel, wxID_ANY);
    auto robotStatuses = simulator_->getRobotStatuses();
    robotGrid->CreateGrid((int)robotStatuses.size(), 5);

    robotGrid->SetColLabelValue(0, "Robot Name");
    robotGrid->SetColLabelValue(1, "Battery Level");
    robotGrid->SetColLabelValue(2, "Water Level");
    robotGrid->SetColLabelValue(3, "Status");
    robotGrid->SetColLabelValue(4, "Current Room");

    robotGrid->SetColSize(0, 100);
    robotGrid->SetColSize(1, 100);
    robotGrid->SetColSize(2, 100);
    robotGrid->SetColSize(3, 120);
    robotGrid->SetColSize(4, 120);

    UpdateRobotGrid();
    sizer->Add(robotGrid, 1, wxEXPAND | wxALL, 5);

    wxButton* refreshBtn = new wxButton(panel, wxID_ANY, "Refresh Status");
    refreshBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnRefreshStatus, this);
    sizer->Add(refreshBtn, 0, wxALL, 5);

    panel->SetSizer(sizer);
    notebook->AddPage(panel, "Dashboard");
}

void RobotManagementFrame::UpdateRobotGrid() {
    auto robotStatuses = simulator_->getRobotStatuses();
    int requiredRows = (int)robotStatuses.size();
    int requiredCols = 5;

    int currentRows = robotGrid->GetNumberRows();
    int currentCols = robotGrid->GetNumberCols();

    if (currentCols != requiredCols) {
        if (currentCols > requiredCols) {
            robotGrid->DeleteCols(0, currentCols - requiredCols);
        } else {
            robotGrid->AppendCols(requiredCols - currentCols);
        }

        robotGrid->SetColLabelValue(0, "Robot Name");
        robotGrid->SetColLabelValue(1, "Battery Level");
        robotGrid->SetColLabelValue(2, "Water Level");
        robotGrid->SetColLabelValue(3, "Status");
        robotGrid->SetColLabelValue(4, "Current Room");

        robotGrid->SetColSize(0, 100);
        robotGrid->SetColSize(1, 100);
        robotGrid->SetColSize(2, 100);
        robotGrid->SetColSize(3, 120);
        robotGrid->SetColSize(4, 120);
    }

    if (currentRows != requiredRows) {
        if (currentRows > requiredRows) {
            robotGrid->DeleteRows(0, currentRows - requiredRows);
        } else {
            robotGrid->AppendRows(requiredRows - currentRows);
        }
    }

    int row = 0;
    for (const auto& robot : robotStatuses) {
        robotGrid->SetCellValue(row, 0, robot.name);
        robotGrid->SetCellValue(row, 1, wxString::Format("%.1f%%", robot.batteryLevel));
        robotGrid->SetCellValue(row, 2, wxString::Format("%.1f%%", robot.waterLevel));
        robotGrid->SetCellValue(row, 3, robot.status);
        robotGrid->SetCellValue(row, 4, robot.currentRoomName);

        if (robot.batteryLevel < 20.0) {
            robotGrid->SetCellBackgroundColour(row, 1, wxColour(255, 200, 200));
        } else {
            robotGrid->SetCellBackgroundColour(row, 1, wxColour(200, 255, 200));
        }

        if (robot.waterLevel < 20.0) {
            robotGrid->SetCellBackgroundColour(row, 2, wxColour(255, 200, 200));
        } else {
            robotGrid->SetCellBackgroundColour(row, 2, wxColour(200, 255, 200));
        }

        row++;
    }

    robotGrid->ForceRefresh();
}

void RobotManagementFrame::CheckAndUpdateAlerts() {
    auto alerts = dbAdapter->retrieveAlerts();
    for (const auto& alert : alerts) {
        std::time_t timestamp = alert.getTimestamp();
        std::string timeStr = std::ctime(&timestamp);
        timeStr = timeStr.substr(0, timeStr.length() - 1);

        wxString alertText = wxString::Format("[%s] %s: %s", 
            timeStr,
            alert.getType(),
            alert.getMessage());

        bool found = false;
        for (unsigned int i = 0; i < alertsList->GetCount(); i++) {
            if (alertsList->GetString(i) == alertText) {
                found = true;
                break;
            }
        }

        if (!found) {
            alertsList->Insert(alertText, 0);
        }
    }
}

void RobotManagementFrame::OnRefreshStatus(wxCommandEvent& evt) {
    UpdateRobotGrid();
}

void RobotManagementFrame::OnClearAlerts(wxCommandEvent& evt) {
    dbAdapter->deleteAllAlerts();
    alertsList->Clear();
}

void RobotManagementFrame::OnRefreshAlerts(wxCommandEvent& evt) {
    if (!dbAdapter) return;
    auto alerts = dbAdapter->retrieveAlerts();
    for (const auto& alert : alerts) {
        std::time_t timestamp = alert.getTimestamp();
        std::string timeStr = std::ctime(&timestamp);
        timeStr = timeStr.substr(0, timeStr.length() - 1);

        wxString alertText = wxString::Format("[%s] %s: %s", 
            timeStr,
            alert.getType(),
            alert.getMessage());

        bool found = false;
        for (unsigned int i = 0; i < alertsList->GetCount(); i++) {
            if (alertsList->GetString(i) == alertText) {
                found = true;
                break;
            }
        }

        if (!found) {
            alertsList->Insert(alertText, 0);
        }
    }
}

void RobotManagementFrame::OnStartCleaning(wxCommandEvent& evt) {
    wxMessageBox("OnStartCleaning invoked. Implement logic here.", "Info");
}

void RobotManagementFrame::OnStopCleaning(wxCommandEvent& evt) {
    wxMessageBox("OnStopCleaning invoked. Implement logic here.", "Info");
}

void RobotManagementFrame::OnReturnToCharger(wxCommandEvent& evt) {
    wxMessageBox("OnReturnToCharger invoked. Implement logic here.", "Info");
}

void RobotManagementFrame::OnStatusUpdateTimer(wxTimerEvent& evt) {
    // Update simulator to move robots
    simulator_->update(1.0f);

    UpdateRobotGrid();
    UpdateRobotChoices();
    UpdateSchedulerRobotChoices();

    if (mapPanel_) {
        mapPanel_->Refresh();
    }
}

void RobotManagementFrame::AddAlert(const Alert& alert) {
    if (dbAdapter) {
        dbAdapter->saveAlertAsync(alert);
    }

    std::time_t timestamp = alert.getTimestamp();
    std::string timeStr = std::ctime(&timestamp);
    timeStr = timeStr.substr(0, timeStr.length() - 1);

    wxString alertText = wxString::Format("[%s] %s: %s", 
        timeStr,
        alert.getType(),
        alert.getMessage());

    alertsList->Insert(alertText, 0);
}

void RobotManagementFrame::BindEvents() {
    Bind(wxEVT_TIMER, &RobotManagementFrame::OnCheckAlerts, this, alertCheckTimer->GetId());
    Bind(wxEVT_TIMER, &RobotManagementFrame::OnStatusUpdateTimer, this, statusUpdateTimer->GetId());
    Bind(wxEVT_MENU, &RobotManagementFrame::OnExit, this, wxID_EXIT);
}

void RobotManagementFrame::OnExit(wxCommandEvent& evt) {
    Close(true);
}

bool RobotManagementFrame::ShowLogin() {
    LoginDialog dlg(this);
    bool loginSuccess = false;

    while (!loginSuccess) {
        if (dlg.ShowModal() == wxID_OK) {
            std::string username = dlg.GetUsername();
            std::string password = dlg.GetPassword();

            auto userIt = std::find_if(users.begin(), users.end(),
                [&username](const std::shared_ptr<User>& user) {
                    return user->getName() == username;
                });

            if (userIt != users.end() && userPasswords[username] == password) {
                currentUser = *userIt;
                loginSuccess = true;
                SetStatusText(wxString::Format("Logged in as: %s (%s)", 
                    username, currentUser->getRole()->getName()), 1);
                return true;
            } else {
                wxMessageBox("Invalid username or password", "Login Error",
                    wxOK | wxICON_ERROR, this);
            }
        } else {
            return false;
        }
    }
    return false;
}

void RobotManagementFrame::CreateUserManagementPanel(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxGrid* userGrid = new wxGrid(panel, wxID_ANY);
    userGrid->CreateGrid((int)users.size(), 2);
    userGrid->SetColLabelValue(0, "Username");
    userGrid->SetColLabelValue(1, "Role");

    for (size_t i = 0; i < users.size(); ++i) {
        userGrid->SetCellValue((int)i, 0, users[i]->getName());
        userGrid->SetCellValue((int)i, 1, users[i]->getRole()->getName());
    }

    sizer->Add(userGrid, 1, wxEXPAND | wxALL, 5);
    panel->SetSizer(sizer);
    notebook->AddPage(panel, "User Management");
}

void RobotManagementFrame::UpdateRobotChoices() {
    // Remove direct calls to robotControlPanel->UpdateRobotList() if it doesn't exist publicly
    UpdateSchedulerRobotChoices();
}

void RobotManagementFrame::UpdateSchedulerRobotChoices() {
    if (schedulerPanel_) {
        schedulerPanel_->UpdateRobotChoices();
    }
}

void RobotManagementFrame::OnAssignTask(wxCommandEvent& event) {
    event.Skip();
}

void RobotManagementFrame::OnRoomSelected(wxCommandEvent& event) {
    event.Skip();
}

void RobotManagementFrame::CreateMapPanel(wxNotebook* notebook) {
    // Already created inline in constructor after permission check
}

void RobotManagementFrame::CreateSchedulerPanel(wxNotebook* notebook) {
    // Already created inline in constructor after permission check
}

void RobotManagementFrame::CreateRobotAnalyticsPanel(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook, wxID_ANY);
    wxStaticText* label = new wxStaticText(panel, wxID_ANY,
        "Robot Analytics Panel under construction", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(label, 1, wxALL | wxEXPAND, 10);

    panel->SetSizer(sizer);
    notebook->AddPage(panel, "Robot Analytics");
}

wxBEGIN_EVENT_TABLE(RobotManagementFrame, wxFrame)
wxEND_EVENT_TABLE()
