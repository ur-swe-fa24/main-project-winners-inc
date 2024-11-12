#include "RobotManagementFrame/RobotManagementFrame.hpp"
#include "LoginDialog/LoginDialog.hpp"
#include "AlertDialog/AlertDialog.hpp"
#include "map_panel/map_panel.hpp"
#include <iostream>
#include <wx/filename.h>


// Constructor
RobotManagementFrame::RobotManagementFrame()
    : wxFrame(nullptr, wxID_ANY, "Robot Management System", wxDefaultPosition, wxSize(800, 600)) {
    try {
        // Initialize MongoDB
        std::string uri = "mongodb://localhost:27017";
        std::string dbName = "mydb9";
        dbAdapter = std::make_shared<MongoDBAdapter>(uri, dbName);

        wxString cwd = wxGetCwd();
        std::cout << "Current working directory: " << cwd.mb_str() << std::endl;

        // Specify the map file path
        std::string mapFile = "map.json"; // Ensurfe map.json is in the working directory

        // Initialize the RobotSimulator with shared dbAdapter and map file
        simulator_ = std::make_unique<RobotSimulator>(dbAdapter, mapFile);
        simulator_->start();

        // Initialize alert system
        alertSystem = std::make_unique<AlertSystem>();

        // Initialize users
        InitializeUsers();

        // Show login dialog
        if (!ShowLogin()) {
            Close();
            return;
        }

        // Create UI components
        CreateMenuBar();

        wxPanel* mainPanel = new wxPanel(this);
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        // Create notebook for tabs
        wxNotebook* notebook = new wxNotebook(mainPanel, wxID_ANY);

        // Create panels
        CreateDashboardPanel(notebook);
        CreateRobotControlPanel(notebook);
        CreateAlertsPanel(notebook);
        CreateMapPanel(notebook); // Create the map panel

        // User Management Panel (only for admin)
        if (currentUser && currentUser->getRole().hasPermission("ADMIN")) {
            CreateUserManagementPanel(notebook);
        }

        mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);
        mainPanel->SetSizer(mainSizer);

        // Create status bar
        CreateStatusBar(2);
        SetStatusText("Ready");
        SetStatusText(wxString::Format("Logged in as: %s", currentUser->getName()), 1);

        // Load data from database
        LoadFromDatabase();

        // Start alert checking timer (every 5 seconds)
        alertTimer = new wxTimer(this, wxID_ANY);
        alertTimer->Start(5000); // 5 seconds

        // Initialize and start the status update timer
        statusUpdateTimer = new wxTimer(this, wxID_ANY);
        statusUpdateTimer->Start(2000); // Refresh every 2 seconds

        BindEvents();

    } catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Initialization failed: %s", e.what()), "Error",
                     wxOK | wxICON_ERROR);
        Close();
        return;
    }
}

// Destructor
RobotManagementFrame::~RobotManagementFrame() {
    if (alertTimer) {
        alertTimer->Stop();
        delete alertTimer;
        alertTimer = nullptr;
    }

    if (statusUpdateTimer) {
        statusUpdateTimer->Stop();
        delete statusUpdateTimer;
        statusUpdateTimer = nullptr;
    }

    if (simulator_) {
        simulator_->stop();
        simulator_.reset();
    }

    if (alertSystem) {
        alertSystem->stop();
        alertSystem.reset();
    }

    // No need to stop threads in dbAdapter, just reset it
    if (dbAdapter) {
        dbAdapter.reset();
    }
}

// Implementations of the missing methods

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
    // Create roles
    Role adminRole("Admin");
    Role userRole("User");
    Role engineerRole("Engineer");

    // Add permissions
    Permission adminPerm("ADMIN");
    Permission userPerm("USER");
    Permission engineerPerm("ENGINEER");

    adminRole.addPermission(adminPerm);
    userRole.addPermission(userPerm);
    engineerRole.addPermission(engineerPerm);

    // Create users with hardcoded passwords
    users.push_back(std::make_shared<User>(1, "admin", adminRole));
    users.push_back(std::make_shared<User>(2, "user", userRole));
    users.push_back(std::make_shared<User>(3, "engineer", engineerRole));

    // Store passwords (in the real system, these would be hashed)
    userPasswords["admin"] = "admin123";
    userPasswords["user"] = "user123";
    userPasswords["engineer"] = "engineer123";
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
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Initialize robotChoice before using it
    robotChoice = new wxChoice(panel, wxID_ANY);

    auto robotStatuses = simulator_->getRobotStatuses();
    for (const auto& status : robotStatuses) {
        robotChoice->Append(status.name);
    }

    // Robot selection
    wxBoxSizer* robotSelectionSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* robotLabel = new wxStaticText(panel, wxID_ANY, "Select Robot:");

    robotSelectionSizer->Add(robotLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    robotSelectionSizer->Add(robotChoice, 1, wxALL, 5);

    // Control buttons
    wxButton* startBtn = new wxButton(panel, wxID_ANY, "Start Cleaning");
    wxButton* stopBtn = new wxButton(panel, wxID_ANY, "Stop Cleaning");
    wxButton* chargeBtn = new wxButton(panel, wxID_ANY, "Return to Charger");

    // Bind button events
    startBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnStartCleaning, this);
    stopBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnStopCleaning, this);
    chargeBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnReturnToCharger, this);

    sizer->Add(robotSelectionSizer, 0, wxEXPAND | wxALL, 5);
    sizer->Add(startBtn, 0, wxEXPAND | wxALL, 5);
    sizer->Add(stopBtn, 0, wxEXPAND | wxALL, 5);
    sizer->Add(chargeBtn, 0, wxEXPAND | wxALL, 5);

    panel->SetSizer(sizer);
    notebook->AddPage(panel, "Robot Control");
}

void RobotManagementFrame::CreateDashboardPanel(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Robot status grid
    robotGrid = new wxGrid(panel, wxID_ANY);
    auto robotStatuses = simulator_->getRobotStatuses();
    robotGrid->CreateGrid(robotStatuses.size(), 4);  // Use 4 columns directly

    // Set column headers
    robotGrid->SetColLabelValue(0, "Robot Name");
    robotGrid->SetColLabelValue(1, "Battery Level");
    robotGrid->SetColLabelValue(2, "Status");
    robotGrid->SetColLabelValue(3, "Current Room");

    UpdateRobotGrid();

    sizer->Add(robotGrid, 1, wxEXPAND | wxALL, 5);

    // Refresh button
    wxButton* refreshBtn = new wxButton(panel, wxID_ANY, "Refresh Status");
    refreshBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnRefreshStatus, this);
    sizer->Add(refreshBtn, 0, wxALL, 5);

    panel->SetSizer(sizer);
    notebook->AddPage(panel, "Dashboard");
}

// void RobotManagementFrame::UpdateRobotGrid() {
//     auto robotStatuses = simulator_->getRobotStatuses();
//     robotGrid->ClearGrid();
//     if (robotGrid->GetNumberRows() != robotStatuses.size()) {
//         robotGrid->DeleteRows(0, robotGrid->GetNumberRows());
//         robotGrid->AppendRows(robotStatuses.size());
//     }
//     for (size_t i = 0; i < robotStatuses.size(); ++i) {
//         robotGrid->SetCellValue(i, 0, robotStatuses[i].name);
//         robotGrid->SetCellValue(i, 1, wxString::Format("%d%%", robotStatuses[i].batteryLevel));
//         robotGrid->SetCellValue(i, 2, robotStatuses[i].isCleaning ? "Cleaning" : "Idle");
//         robotGrid->SetCellValue(i, 3, robotStatuses[i].currentRoomName);

//         // Color coding for battery levels
//         if (robotStatuses[i].batteryLevel < 20) {
//             robotGrid->SetCellBackgroundColour(i, 1, wxColour(255, 200, 200)); // Light red
//         } else if (robotStatuses[i].batteryLevel < 50) {
//             robotGrid->SetCellBackgroundColour(i, 1, wxColour(255, 255, 200)); // Light yellow
//         } else {
//             robotGrid->SetCellBackgroundColour(i, 1, wxColour(200, 255, 200)); // Light green
//         }
//     }
//     robotGrid->AutoSize();
// }


void RobotManagementFrame::UpdateRobotGrid() {
    auto robotStatuses = simulator_->getRobotStatuses();
    int requiredRows = robotStatuses.size();
    int requiredCols = 4;  // Number of columns you have

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
        robotGrid->SetColLabelValue(2, "Status");
        robotGrid->SetColLabelValue(3, "Current Room");
    }

    // Adjust the number of rows
    if (currentRows != requiredRows) {
        if (currentRows > requiredRows) {
            robotGrid->DeleteRows(0, currentRows - requiredRows);
        } else {
            robotGrid->AppendRows(requiredRows - currentRows);
        }
    }

    // Now, set the cell values
    for (size_t i = 0; i < robotStatuses.size(); ++i) {
        robotGrid->SetCellValue(i, 0, robotStatuses[i].name);
        robotGrid->SetCellValue(i, 1, wxString::Format("%d%%", robotStatuses[i].batteryLevel));
        robotGrid->SetCellValue(i, 2, robotStatuses[i].isCleaning ? "Cleaning" : "Idle");
        robotGrid->SetCellValue(i, 3, robotStatuses[i].currentRoomName);

        // Color coding for battery levels
        if (robotStatuses[i].batteryLevel < 20) {
            robotGrid->SetCellBackgroundColour(i, 1, wxColour(255, 200, 200)); // Light red
        } else if (robotStatuses[i].batteryLevel < 50) {
            robotGrid->SetCellBackgroundColour(i, 1, wxColour(255, 255, 200)); // Light yellow
        } else {
            robotGrid->SetCellBackgroundColour(i, 1, wxColour(200, 255, 200)); // Light green
        }
    }

    robotGrid->AutoSize();
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
    // Update robots from simulator
    // robots = simulator_->getRobotStatuses();
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
    if (robotChoice && robotChoice->GetSelection() != wxNOT_FOUND) {
        std::string robotName = robotChoice->GetString(robotChoice->GetSelection()).ToStdString();

        // Start cleaning
        simulator_->startCleaning(robotName);

        // Create and save "Started Cleaning" alert
        auto robot = simulator_->getRobotByName(robotName);
        auto room = std::make_shared<Room>("Cleaning Area", 1);
        auto alert = std::make_shared<Alert>("Started Cleaning", "Robot " + robotName + " has started cleaning",
                                             robot, room, std::time(nullptr));
        alertSystem->sendAlert(currentUser.get(), alert);
        dbAdapter->saveAlert(*alert);
    }
}

void RobotManagementFrame::OnStopCleaning(wxCommandEvent& evt) {
    if (robotChoice && robotChoice->GetSelection() != wxNOT_FOUND) {
        std::string robotName = robotChoice->GetString(robotChoice->GetSelection()).ToStdString();

        // Stop cleaning
        simulator_->stopCleaning(robotName);

        // Create and save "Stopped Cleaning" alert
        auto robot = simulator_->getRobotByName(robotName);
        auto room = std::make_shared<Room>("Cleaning Area", 1);
        auto alert = std::make_shared<Alert>("Stopped Cleaning", "Robot " + robotName + " has stopped cleaning",
                                             robot, room, std::time(nullptr));
        alertSystem->sendAlert(currentUser.get(), alert);
        dbAdapter->saveAlert(*alert);
    }
}

void RobotManagementFrame::OnReturnToCharger(wxCommandEvent& evt) {
    if (robotChoice && robotChoice->GetSelection() != wxNOT_FOUND) {
        std::string robotName = robotChoice->GetString(robotChoice->GetSelection()).ToStdString();

        // Return to charger
        simulator_->returnToCharger(robotName);

        // Create and save "Charging" alert
        auto robot = simulator_->getRobotByName(robotName);
        auto room = std::make_shared<Room>("Charging Station", 0);
        auto alert = std::make_shared<Alert>("Charging", "Robot " + robotName + " has returned to charger",
                                             robot, room, std::time(nullptr));
        alertSystem->sendAlert(currentUser.get(), alert);
        dbAdapter->saveAlert(*alert);

        UpdateRobotGrid();
    }
}

// void RobotManagementFrame::OnStatusUpdateTimer(wxTimerEvent& evt) {
//     // robots = simulator_->getRobotStatuses();
//     UpdateRobotGrid();
// }

void RobotManagementFrame::BindEvents() {
    // Bind the timer event for checking alerts
    Bind(wxEVT_TIMER, &RobotManagementFrame::OnCheckAlerts, this, alertTimer->GetId());

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
    if (dlg.ShowModal() == wxID_OK) {
        std::string username = dlg.GetUsername();
        std::string password = dlg.GetPassword();

        // Check credentials
        if (userPasswords.find(username) != userPasswords.end() && userPasswords[username] == password) {
            // Find user
            for (const auto& user : users) {
                if (user->getName() == username) {
                    currentUser = user;
                    return true;
                }
            }
        }
        wxMessageBox("Invalid credentials!", "Error", wxOK | wxICON_ERROR);
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
        userGrid->SetCellValue(i, 1, users[i]->getRole().getName());
    }

    sizer->Add(userGrid, 1, wxEXPAND | wxALL, 5);
    panel->SetSizer(sizer);
    notebook->AddPage(panel, "User Management");
}


void RobotManagementFrame::CreateMapPanel(wxNotebook* notebook) {
    wxPanel* panel = new wxPanel(notebook);

    // Get robots from simulator
    const std::vector<std::shared_ptr<Robot>>& robots = simulator_->getRobots();

    // Create MapPanel
    mapPanel_ = new MapPanel(panel, simulator_->getMap(), robots);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(mapPanel_, 1, wxEXPAND | wxALL, 5);
    panel->SetSizer(sizer);
    notebook->AddPage(panel, "Map");
}

void RobotManagementFrame::OnStatusUpdateTimer(wxTimerEvent& evt) {
    // Update robot statuses
    UpdateRobotGrid();

    // Refresh the map panel
    if (mapPanel_) {
        mapPanel_->Refresh();
    }
}


