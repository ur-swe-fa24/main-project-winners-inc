#include "RobotManagementFrame/RobotManagementFrame.hpp"
#include "LoginDialog/LoginDialog.hpp"
#include "AlertDialog/AlertDialog.hpp"
#include "map_panel/map_panel.hpp"
#include "config/ResourceConfig.hpp"
#include <wx/notebook.h>
#include <wx/grid.h>
#include <iostream>
#include <wx/filename.h>

const std::string RobotManagementFrame::DB_URI = "mongodb://localhost:27017";
const std::string RobotManagementFrame::DB_NAME = "mydb9";

// Constructor
RobotManagementFrame::RobotManagementFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1024, 768)),
      robotControlPanel(nullptr)  // Initialize robotControlPanel to nullptr
{
    try {
        // Create UI components first
        CreateMenuBar();
        CreateStatusBar(2);  // Create status bar with 2 fields
        SetStatusText("Initializing...");  // Now safe to set status

        // Initialize resource configuration
        config::ResourceConfig::initialize();

        // Get current working directory
        wxString cwd = wxGetCwd();
        std::cout << "Current working directory: " << cwd.mb_str() << std::endl;

        // Initialize database connection
        try {
            dbAdapter = std::make_shared<MongoDBAdapter>(DB_URI, DB_NAME);
            SetStatusText("Connected to database");
        } catch (const std::exception& e) {
            wxMessageBox(wxString::Format("Failed to connect to database: %s", e.what()),
                "Database Error", wxOK | wxICON_ERROR);
            Close(true);
            return;
        }

        // Initialize simulator
        try {
            simulator_ = std::make_unique<RobotSimulator>(dbAdapter, config::ResourceConfig::getMapPath());
            simulator_->start();  // Start the simulator
            SetStatusText("Simulator started");
        } catch (const std::exception& e) {
            wxMessageBox(wxString::Format("Failed to initialize simulator: %s", e.what()),
                "Simulator Error", wxOK | wxICON_ERROR);
            Close(true);
            return;
        }

        // Initialize scheduler with simulator data
        scheduler_ = Scheduler(&simulator_->getMap(), &simulator_->getRobots());

        // Initialize alert system
        alertSystem = std::make_unique<AlertSystem>();

        // Initialize users and show login dialog
        InitializeUsers();
        if (!ShowLogin()) {
            Close(true);
            return;
        }

        // Create the notebook
        wxNotebook* notebook = new wxNotebook(this, wxID_ANY);
        SetSizer(new wxBoxSizer(wxVERTICAL));
        GetSizer()->Add(notebook, 1, wxEXPAND);

        // Create panels according to user permissions
        if (currentUser->getRole()->hasPermission("Dashboard")) {
            CreateDashboardPanel(notebook);
        }

        if (currentUser->getRole()->hasPermission("Scheduler")) {
            CreateSchedulerPanel(notebook);
        }

        if (currentUser->getRole()->hasPermission("Alert")) {
            CreateAlertsPanel(notebook);
        }

        if (currentUser->getRole()->hasPermission("Map")) {
            CreateMapPanel(notebook);
        }

        if (currentUser->getRole()->hasPermission("Robot Control")) {
            CreateRobotControlPanel(notebook);
        }

        if (currentUser->getRole()->hasPermission("Robot Analytics")) {
            CreateRobotAnalyticsPanel(notebook);
        }

        // User Management Panel (only for admin)
        if (currentUser && currentUser->getRole()->hasPermission("ADMIN")) {
            CreateUserManagementPanel(notebook);
        }

        // Initialize alert check timer
        alertCheckTimer = new wxTimer(this, ALERT_TIMER_ID);
        alertCheckTimer->Start(5000); // Check every 5 seconds

        // Initialize status update timer
        statusUpdateTimer = new wxTimer(this, STATUS_TIMER_ID);
        statusUpdateTimer->Start(1000); // Update every second

        // Bind events
        BindEvents();

        // Set final status
        SetStatusText("Ready");
        SetStatusText(wxString::Format("Logged in as: %s (%s)", 
            currentUser->getName(), currentUser->getRole()->getName()), 1);

        // Update robot choices
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

// Destructor
RobotManagementFrame::~RobotManagementFrame() {
    // Clean up timers
    if (statusUpdateTimer) {
        statusUpdateTimer->Stop();
        delete statusUpdateTimer;
    }
    if (alertCheckTimer) {
        alertCheckTimer->Stop();
        delete alertCheckTimer;
    }

    // Clean up robotControlPanel (will be deleted by wxWidgets since it's a window)
    robotControlPanel = nullptr;

    // Clean up simulator
    if (simulator_) {
        simulator_->stop();
    }
}

// Implementations of the methods

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
    // Create roles with specific permissions
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

    // Create users
    users.push_back(std::make_shared<User>("building_manager", buildingManagerRole));
    users.push_back(std::make_shared<User>("field_engineer", fieldEngineerRole));
    users.push_back(std::make_shared<User>("senior_manager", seniorManagerRole));

    // Store passwords (in a real system, these would be hashed)
    userPasswords["building_manager"] = "bm123";
    userPasswords["field_engineer"] = "fe123";
    userPasswords["senior_manager"] = "sm123";
}

void RobotManagementFrame::LoadFromDatabase() {
    // Load alerts
    auto alerts = dbAdapter->retrieveAlerts();
    for (const auto& alert : alerts) {
        alertsList->Append(alert.getMessage());
    }

    UpdateRobotGrid();
}

void RobotManagementFrame::CreateAlertsPanel(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Alerts list
    alertsList = new wxListBox(panel, wxID_ANY);
    sizer->Add(alertsList, 1, wxEXPAND | wxALL, 5);

    // Button sizer for multiple buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    // Clear alerts button
    wxButton* clearBtn = new wxButton(panel, wxID_ANY, "Clear Alerts");
    clearBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnClearAlerts, this);
    buttonSizer->Add(clearBtn, 1, wxALL, 5);

    // Refresh alerts button
    wxButton* refreshBtn = new wxButton(panel, wxID_ANY, "Refresh Alerts");
    refreshBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnRefreshAlerts, this);
    buttonSizer->Add(refreshBtn, 1, wxALL, 5);

    sizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
    panel->SetSizer(sizer);
    notebook->AddPage(panel, "Alerts");
}

void RobotManagementFrame::CreateRobotControlPanel(wxNotebook* notebook) {
    robotControlPanel = new RobotControlPanel(notebook, simulator_.get());
    notebook->AddPage(robotControlPanel, "Robot Control");
}

void RobotManagementFrame::CreateDashboardPanel(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Robot status grid
    robotGrid = new wxGrid(panel, wxID_ANY);
    auto robotStatuses = simulator_->getRobotStatuses();
    robotGrid->CreateGrid(robotStatuses.size(), 5);  // Added column for water level

    // Set column headers
    robotGrid->SetColLabelValue(0, "Robot Name");
    robotGrid->SetColLabelValue(1, "Battery Level");
    robotGrid->SetColLabelValue(2, "Water Level");  // New column
    robotGrid->SetColLabelValue(3, "Status");
    robotGrid->SetColLabelValue(4, "Current Room");

    // Set column widths
    robotGrid->SetColSize(0, 100);  // Robot Name
    robotGrid->SetColSize(1, 100);  // Battery Level
    robotGrid->SetColSize(2, 100);  // Water Level
    robotGrid->SetColSize(3, 120);  // Status
    robotGrid->SetColSize(4, 120);  // Current Room

    UpdateRobotGrid();

    sizer->Add(robotGrid, 1, wxEXPAND | wxALL, 5);

    // Refresh button
    wxButton* refreshBtn = new wxButton(panel, wxID_ANY, "Refresh Status");
    refreshBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnRefreshStatus, this);
    sizer->Add(refreshBtn, 0, wxALL, 5);

    panel->SetSizer(sizer);
    notebook->AddPage(panel, "Dashboard");
}

void RobotManagementFrame::UpdateRobotGrid() {
    auto robotStatuses = simulator_->getRobotStatuses();
    int requiredRows = robotStatuses.size();
    int requiredCols = 5;  // Updated for water level column

    int currentRows = robotGrid->GetNumberRows();
    int currentCols = robotGrid->GetNumberCols();

    // Adjust the number of columns if necessary
    if (currentCols != requiredCols) {
        if (currentCols > requiredCols) {
            robotGrid->DeleteCols(0, currentCols - requiredCols);
        } else {
            robotGrid->AppendCols(requiredCols - currentCols);
        }
        // Reset column labels after adjusting columns
        robotGrid->SetColLabelValue(0, "Robot Name");
        robotGrid->SetColLabelValue(1, "Battery Level");
        robotGrid->SetColLabelValue(2, "Water Level");
        robotGrid->SetColLabelValue(3, "Status");
        robotGrid->SetColLabelValue(4, "Current Room");

        // Set column widths
        robotGrid->SetColSize(0, 100);  // Robot Name
        robotGrid->SetColSize(1, 100);  // Battery Level
        robotGrid->SetColSize(2, 100);  // Water Level
        robotGrid->SetColSize(3, 120);  // Status
        robotGrid->SetColSize(4, 120);  // Current Room
    }

    // Adjust the number of rows if necessary
    if (currentRows != requiredRows) {
        if (currentRows > requiredRows) {
            robotGrid->DeleteRows(0, currentRows - requiredRows);
        } else {
            robotGrid->AppendRows(requiredRows - currentRows);
        }
    }

    // Update the grid with robot statuses
    int row = 0;
    for (const auto& robot : robotStatuses) {
        robotGrid->SetCellValue(row, 0, robot.name);
        robotGrid->SetCellValue(row, 1, wxString::Format("%d%%", robot.batteryLevel));
        robotGrid->SetCellValue(row, 2, wxString::Format("%d%%", robot.waterLevel));  // New water level display
        robotGrid->SetCellValue(row, 3, robot.status);
        robotGrid->SetCellValue(row, 4, robot.currentRoomName);

        // Color coding for battery and water levels
        if (robot.batteryLevel < 20) {
            robotGrid->SetCellBackgroundColour(row, 1, wxColour(255, 200, 200));  // Light red for low battery
        } else {
            robotGrid->SetCellBackgroundColour(row, 1, wxColour(200, 255, 200));  // Light green for good battery
        }

        if (robot.waterLevel < 20) {
            robotGrid->SetCellBackgroundColour(row, 2, wxColour(255, 200, 200));  // Light red for low water
        } else {
            robotGrid->SetCellBackgroundColour(row, 2, wxColour(200, 255, 200));  // Light green for good water level
        }

        row++;
    }

    robotGrid->ForceRefresh();
}

void RobotManagementFrame::CheckAndUpdateAlerts() {
    // Refresh the alerts from database
    alertsList->Clear();
    auto alerts = dbAdapter->retrieveAlerts();
    for (const auto& alert : alerts) {
        alertsList->Append(alert.getMessage());
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
    CheckAndUpdateAlerts();
}

void RobotManagementFrame::OnStartCleaning(wxCommandEvent& evt) {
    if (robotControlPanel) {
        robotControlPanel->OnStartCleaning(evt);
    }
}

void RobotManagementFrame::OnStopCleaning(wxCommandEvent& evt) {
    if (robotControlPanel) {
        robotControlPanel->OnStopCleaning(evt);
    }
}

void RobotManagementFrame::OnReturnToCharger(wxCommandEvent& evt) {
    if (robotControlPanel) {
        robotControlPanel->OnReturnToCharger(evt);
    }
}

void RobotManagementFrame::OnStatusUpdateTimer(wxTimerEvent& evt) {
    // Update robot statuses
    UpdateRobotGrid();

    // Refresh the robot choices in case robots were added/deleted
    UpdateRobotChoices();
    UpdateSchedulerRobotChoices(); // Ensure this method exists and works correctly

    // Refresh the map panel to show updated robot positions
    if (mapPanel_) {
        mapPanel_->Refresh();
    }
}


void RobotManagementFrame::BindEvents() {
    // Bind the timer event for checking alerts
    Bind(wxEVT_TIMER, &RobotManagementFrame::OnCheckAlerts, this, alertCheckTimer->GetId());

    // Bind the timer event for status updates
    Bind(wxEVT_TIMER, &RobotManagementFrame::OnStatusUpdateTimer, this, statusUpdateTimer->GetId());

    // Bind the menu event for exiting the application
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

            // Find user
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
            // User clicked Cancel
            return false;
        }
    }
    return false;
}

void RobotManagementFrame::CreateUserManagementPanel(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Users grid
    wxGrid* userGrid = new wxGrid(panel, wxID_ANY);
    userGrid->CreateGrid(users.size(), 2);

    // Set column headers
    userGrid->SetColLabelValue(0, "Username");
    userGrid->SetColLabelValue(1, "Role");

    // Fill user data
    for (size_t i = 0; i < users.size(); ++i) {
        userGrid->SetCellValue(i, 0, users[i]->getName());
        userGrid->SetCellValue(i, 1, users[i]->getRole()->getName());
    }

    sizer->Add(userGrid, 1, wxEXPAND | wxALL, 5);

    panel->SetSizer(sizer);
    notebook->AddPage(panel, "User Management");
}

void RobotManagementFrame::CreateMapPanel(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook);

    // Create MapPanel, passing simulator_
    mapPanel_ = new MapPanel(panel, simulator_->getMap(), simulator_.get());

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(mapPanel_, 1, wxEXPAND | wxALL, 5);
    panel->SetSizer(sizer);
    notebook->AddPage(panel, "Map");
}

void RobotManagementFrame::UpdateRobotChoices() {
    if (!simulator_) {
        wxLogError("Simulator is not initialized");
        return;
    }

    try {
        // Only update robot control panel if it exists (i.e., user has robot control access)
        if (robotControlPanel) {
            robotControlPanel->UpdateRobotList();
        }
        
        // Always update scheduler robot choices as it's available to all users
        UpdateSchedulerRobotChoices();
    } catch (const std::exception& e) {
        wxLogError("Error updating robot choices: %s", e.what());
    } catch (...) {
        wxLogError("Unknown error occurred while updating robot choices");
    }
}

void RobotManagementFrame::UpdateSchedulerRobotChoices() {
    if (!simulator_) {
        wxLogError("Simulator is not initialized");
        return;
    }

    if (schedulerRobotChoice) {
        try {
            wxString currentSelection = schedulerRobotChoice->GetStringSelection();

            schedulerRobotChoice->Clear();
            auto robotStatuses = simulator_->getRobotStatuses();
            
            for (const auto& status : robotStatuses) {
                if (!status.name.empty()) {  // Add check for empty name
                    schedulerRobotChoice->Append(status.name);
                }
            }

            // Restore the selection
            int index = schedulerRobotChoice->FindString(currentSelection);
            if (index != wxNOT_FOUND) {
                schedulerRobotChoice->SetSelection(index);
            } else if (schedulerRobotChoice->GetCount() > 0) {
                schedulerRobotChoice->SetSelection(0);
            }

            schedulerRobotChoice->Refresh();
        } catch (const std::exception& e) {
            wxLogError("Error updating scheduler robot choices: %s", e.what());
        } catch (...) {
            wxLogError("Unknown error occurred while updating scheduler robot choices");
        }
    }
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

void RobotManagementFrame::CreateSchedulerPanel(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* robotLabel = new wxStaticText(panel, wxID_ANY, "Select Robot:");
    schedulerRobotChoice = new wxChoice(panel, wxID_ANY); // Assign to member variable

    UpdateSchedulerRobotChoices();

    // Populate schedulerRobotChoice with robot names
    // if (robotChoice) {
    //     for (unsigned int i = 0; i < robotChoice->GetCount(); ++i) {
    //         schedulerRobotChoice->Append(robotChoice->GetString(i));
    //     }
    // }

    wxStaticText* roomLabel = new wxStaticText(panel, wxID_ANY, "Enter Room ID:");
    roomIdInput = new wxTextCtrl(panel, wxID_ANY);

    wxStaticText* strategyLabel = new wxStaticText(panel, wxID_ANY, "Cleaning Strategy:");
    strategyChoice = new wxChoice(panel, wxID_ANY);
    strategyChoice->Append("Standard");
    strategyChoice->Append("Deep Clean");

    wxButton* assignTaskBtn = new wxButton(panel, wxID_ANY, "Assign Task");
    assignTaskBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnAssignTask, this);

    sizer->Add(robotLabel, 0, wxALL, 5);
    sizer->Add(schedulerRobotChoice, 0, wxEXPAND | wxALL, 5);
    sizer->Add(roomLabel, 0, wxALL, 5);
    sizer->Add(roomIdInput, 0, wxEXPAND | wxALL, 5);
    sizer->Add(strategyLabel, 0, wxALL, 5);
    sizer->Add(strategyChoice, 0, wxEXPAND | wxALL, 5);
    sizer->Add(assignTaskBtn, 0, wxEXPAND | wxALL, 5);

    panel->SetSizer(sizer);
    notebook->AddPage(panel, "Scheduler");
}

void RobotManagementFrame::OnAssignTask(wxCommandEvent& event) {
    if (!schedulerRobotChoice || schedulerRobotChoice->GetSelection() == wxNOT_FOUND) {
        wxMessageBox("Please select a robot from the Scheduler panel.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    std::string robotName = schedulerRobotChoice->GetStringSelection().ToStdString();

    if (roomIdInput->GetValue().IsEmpty()) {
        wxMessageBox("Please enter a room ID.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    int roomId;
    try {
        roomId = std::stoi(roomIdInput->GetValue().ToStdString());
    } catch (const std::invalid_argument&) {
        wxMessageBox("Invalid room ID. Please enter a numeric value.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    if (strategyChoice->GetSelection() == wxNOT_FOUND) {
        wxMessageBox("Please select a cleaning strategy.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    std::string strategy = strategyChoice->GetStringSelection().ToStdString();

    scheduler_.assignCleaningTask(robotName, roomId, strategy);

    // Log the task assignment
    std::cout << "Task assigned: Robot " << robotName << " to clean Room " << roomId << " with strategy " << strategy << "." << std::endl;

    // Retrieve the target room from the map
    Room* targetRoom = simulator_->getMap().getRoomById(roomId);
    if (!targetRoom) {
        wxMessageBox("Target room does not exist in the map.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // Create and save "Assigned Cleaning Task" alert using existing Room instance
    auto robot = simulator_->getRobotByName(robotName);
    if (!robot) {
        wxMessageBox("Robot not found.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    auto alert = std::make_shared<Alert>(
        "Assigned Cleaning Task",
        "Robot " + robotName + " has been assigned to clean " + targetRoom->getRoomName(),
        robot,
        std::make_shared<Room>(*targetRoom),
        std::time(nullptr)
    );
    alertSystem->sendAlert(currentUser.get(), alert);
    dbAdapter->saveAlert(*alert);

    mapPanel_->Refresh(); // Update UI
    UpdateRobotGrid();    // Update robot status grid
}

wxBEGIN_EVENT_TABLE(RobotManagementFrame, wxFrame)
    // Event table entries (if any)
wxEND_EVENT_TABLE()
