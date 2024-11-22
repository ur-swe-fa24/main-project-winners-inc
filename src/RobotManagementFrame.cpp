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
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
      dbAdapter(std::make_shared<MongoDBAdapter>(DB_URI, DB_NAME)),
      simulator_(std::make_unique<RobotSimulator>(dbAdapter, config::ResourceConfig::getMapPath())),
      scheduler_(simulator_->getMap(), simulator_->getRobots())
{
    try {
        // Initialize resource configuration
        config::ResourceConfig::initialize();
        
        // Start the simulator
        simulator_->start();

        wxString cwd = wxGetCwd();
        std::cout << "Current working directory: " << cwd.mb_str() << std::endl;

        // Initialize alert system
        alertSystem = std::make_unique<AlertSystem>();

        // Initialize users
        InitializeUsers();

        // Add initial robots
        simulator_->addRobot("Cleaner1");
        simulator_->addRobot("Cleaner2");
        simulator_->addRobot("Cleaner3");
        std::cout << "Added initial robots to the system" << std::endl;

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
        CreateSchedulerPanel(notebook); // Create the scheduler panel

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
        statusUpdateTimer->Start(1000); // Refresh every second

        BindEvents();

        // Update robot choices after adding predefined robots
        UpdateRobotChoices();
        UpdateSchedulerRobotChoices();
        UpdateRobotGrid();
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

    // Robot selection
    wxBoxSizer* robotSelectionSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* robotLabel = new wxStaticText(panel, wxID_ANY, "Select Robot:");

    robotChoice = new wxChoice(panel, wxID_ANY);
    UpdateRobotChoices();  // Initialize robot choices

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

    // Add control buttons to sizer
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
        robotGrid->SetCellValue(i, 2, robotStatuses[i].status);
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

        // Get the robot instance
        auto robot = simulator_->getRobotByName(robotName);
        if (!robot) {
            wxMessageBox("Selected robot not found.", "Error", wxOK | wxICON_ERROR);
            return;
        }

        if (robot->isCharging()) {
            wxMessageBox("Robot is already charging.", "Info", wxOK | wxICON_INFORMATION);
            return;
        }

        // Return to charger
        simulator_->returnToCharger(robotName);

        // Create and save "Charging" alert
        auto room = std::make_shared<Room>("Charging Station", 0);
        auto alert = std::make_shared<Alert>("Charging", "Robot " + robotName + " is returning to charger",
                                             robot, room, std::time(nullptr));
        alertSystem->sendAlert(currentUser.get(), alert);
        dbAdapter->saveAlert(*alert);

        UpdateRobotGrid();

        wxMessageBox("Robot is returning to charger.", "Info", wxOK | wxICON_INFORMATION);
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

    // Create MapPanel, passing simulator_
    mapPanel_ = new MapPanel(panel, simulator_->getMap(), simulator_.get());

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(mapPanel_, 1, wxEXPAND | wxALL, 5);
    panel->SetSizer(sizer);
    notebook->AddPage(panel, "Map");
}

void RobotManagementFrame::UpdateRobotChoices() {
    if (robotChoice) {
        // Save the currently selected robot name
        wxString currentSelection = robotChoice->GetStringSelection();

        robotChoice->Clear();
        auto robotStatuses = simulator_->getRobotStatuses();
        std::cout << "Updating Robot Control Choices:" << std::endl;
        for (const auto& status : robotStatuses) {
            robotChoice->Append(status.name);
            std::cout << " - " << status.name << std::endl;
        }

        // Restore the selection
        int index = robotChoice->FindString(currentSelection);
        if (index != wxNOT_FOUND) {
            robotChoice->SetSelection(index);
        } else if (robotChoice->GetCount() > 0) {
            robotChoice->SetSelection(0); // Optionally select the first item
        }

        robotChoice->Refresh(); // Ensure the UI reflects changes
    }
}

void RobotManagementFrame::UpdateSchedulerRobotChoices() {
    if (schedulerRobotChoice) {
        // Save the currently selected robot name
        wxString currentSelection = schedulerRobotChoice->GetStringSelection();

        schedulerRobotChoice->Clear();
        auto robotStatuses = simulator_->getRobotStatuses();
        std::cout << "Updating Scheduler Robot Choices:" << std::endl;
        for (const auto& status : robotStatuses) {
            schedulerRobotChoice->Append(status.name);
            std::cout << " - " << status.name << std::endl;
        }

        // Restore the selection
        int index = schedulerRobotChoice->FindString(currentSelection);
        if (index != wxNOT_FOUND) {
            schedulerRobotChoice->SetSelection(index);
        } else if (schedulerRobotChoice->GetCount() > 0) {
            schedulerRobotChoice->SetSelection(0); // Optionally select the first item
        }

        schedulerRobotChoice->Refresh(); // Ensure the UI reflects changes
    }
}



// void RobotManagementFrame::OnAddRobot(wxCommandEvent& evt) {
//     wxTextEntryDialog dlg(this, "Enter new robot name:", "Add Robot");
//     if (dlg.ShowModal() == wxID_OK) {
//         std::string robotName = dlg.GetValue().ToStdString();
//         try {
//             simulator_->addRobot(robotName);

//             // Update UI
//             UpdateRobotChoices();
//             UpdateSchedulerRobotChoices(); // Add this line

//             UpdateRobotGrid();

//             // Optionally, create an alert for the new robot
//             auto robot = simulator_->getRobotByName(robotName);
//             auto room = std::make_shared<Room>("Starting Area", 1);
//             auto alert = std::make_shared<Alert>("New Robot Added", "Robot " + robotName + " has been added",
//                                                  robot, room, std::time(nullptr));
//             alertSystem->sendAlert(currentUser.get(), alert);
//             dbAdapter->saveAlert(*alert);
//         } catch (const std::exception& e) {
//             wxMessageBox(wxString::Format("Failed to add robot: %s", e.what()), "Error",
//                          wxOK | wxICON_ERROR);
//         }
//     }
// }

// void RobotManagementFrame::OnDeleteRobot(wxCommandEvent& evt) {
//     if (robotChoice && robotChoice->GetSelection() != wxNOT_FOUND) {
//         std::string robotName = robotChoice->GetString(robotChoice->GetSelection()).ToStdString();
//         int response = wxMessageBox(wxString::Format("Are you sure you want to delete robot '%s'?", robotName),
//                                     "Confirm Deletion", wxYES_NO | wxICON_QUESTION);
//         if (response == wxYES) {
//             try {
//                 simulator_->deleteRobot(robotName);

//                 // Update UI
//                 UpdateRobotChoices();
//                 UpdateSchedulerRobotChoices(); // Add this line
//                 UpdateRobotGrid();

//                 // Optionally, create an alert for robot deletion
//                 auto alert = std::make_shared<Alert>("Robot Deleted", "Robot " + robotName + " has been deleted",
//                                                      nullptr, nullptr, std::time(nullptr));
//                 alertSystem->sendAlert(currentUser.get(), alert);
//                 dbAdapter->saveAlert(*alert);
//             } catch (const std::exception& e) {
//                 wxMessageBox(wxString::Format("Failed to delete robot: %s", e.what()), "Error",
//                              wxOK | wxICON_ERROR);
//             }
//         }
//     } else {
//         wxMessageBox("Please select a robot to delete.", "Error", wxOK | wxICON_ERROR);
//     }
// }

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
