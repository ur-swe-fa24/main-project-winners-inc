#include "RobotManagementFrame/RobotManagementFrame.hpp"

// Implement the methods as per your existing code, with modifications to integrate the simulator

RobotManagementFrame::RobotManagementFrame()
    : wxFrame(nullptr, wxID_ANY, "Robot Management System", wxDefaultPosition, wxSize(800, 600)) {
    try {
        Bind(wxEVT_TIMER, &RobotManagementFrame::OnCheckAlerts, this);
        Bind(wxEVT_MENU, &RobotManagementFrame::OnExit, this, wxID_EXIT);
        Bind(wxEVT_TIMER, &RobotManagementFrame::OnCheckAlerts, this);

        // Initialize MongoDB
        std::string uri = "mongodb://localhost:27017";
        std::string dbName = "mydb8";
        dbAdapter = std::make_unique<MongoDBAdapter>(uri, dbName);

        // Initialize users and roles
        InitializeUsers();

        // Show login dialog
        if (!ShowLogin()) {
            Close();
            return;
        }

        // Initialize simulator
        simulator_ = std::make_unique<RobotSimulator>(dbAdapter);
        simulator_->start();

        // Load robots from simulator
        robots = simulator_->getRobots();

        // Create main panel with notebook for tabs
        wxPanel* mainPanel = new wxPanel(this);
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        // Create notebook for tabs
        wxNotebook* notebook = new wxNotebook(mainPanel, wxID_ANY);

        // Create panels
        CreateDashboardPanel(notebook);
        CreateRobotControlPanel(notebook);
        CreateAlertsPanel(notebook);

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

        // Create menu bar
        CreateMenuBar();

        // Start alert checking timer (every 5 seconds)
        alertTimer = new wxTimer(this);
        alertTimer->Start(5000); // 5 seconds

        // Create alert system
        alertSystem = std::make_unique<AlertSystem>();

        // Load existing alerts from MongoDB
        LoadFromDatabase();
    } catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Database initialization failed: %s", e.what()), "Error",
                     wxOK | wxICON_ERROR);
        Close();
        return;
    }
}

RobotManagementFrame::~RobotManagementFrame() {
    if (alertTimer) {
        alertTimer->Stop();
        delete alertTimer;
        alertTimer = nullptr;
    }

    if (alertSystem) {
        alertSystem->stop();
        alertSystem.reset();
    }

    if (dbAdapter) {
        dbAdapter->stop();
        dbAdapter->stopRobotStatusThread();
        dbAdapter.reset();
    }

    if (simulator_) {
        simulator_->stop();
        simulator_.reset();
    }
}

// Implement other methods, updating them to interact with the simulator

void RobotManagementFrame::OnRefreshStatus(wxCommandEvent& evt) {
    // Get updated robots from simulator
    robots = simulator_->getRobots();

    // Update robot grid
    UpdateRobotGrid();
}

void RobotManagementFrame::UpdateRobotGrid() {
    robots = simulator_->getRobots();
    robotGrid->ClearGrid();

    // Ensure the grid has enough rows
    if (robotGrid->GetNumberRows() != robots.size()) {
        robotGrid->DeleteRows(0, robotGrid->GetNumberRows());
        robotGrid->AppendRows(robots.size());
    }

    for (size_t i = 0; i < robots.size(); ++i) {
        robotGrid->SetCellValue(i, 0, robots[i]->getName());
        robotGrid->SetCellValue(i, 1, wxString::Format("%d%%", robots[i]->getBatteryLevel()));
        robotGrid->SetCellValue(i, 2, robots[i]->isCleaning() ? "Cleaning" : "Idle");

        // Color coding for battery levels
        if (robots[i]->getBatteryLevel() < 20) {
            robotGrid->SetCellBackgroundColour(i, 1, wxColour(255, 200, 200)); // Light red
        } else if (robots[i]->getBatteryLevel() < 50) {
            robotGrid->SetCellBackgroundColour(i, 1, wxColour(255, 255, 200)); // Light yellow
        } else {
            robotGrid->SetCellBackgroundColour(i, 1, wxColour(200, 255, 200)); // Light green
        }
    }
    robotGrid->AutoSize();
    robotGrid->ForceRefresh();
}

void RobotManagementFrame::OnStartCleaning(wxCommandEvent& evt) {
    wxChoice* choice = dynamic_cast<wxChoice*>(FindWindow(wxID_ANY));
    if (choice && choice->GetSelection() != wxNOT_FOUND) {
        std::string robotName = choice->GetString(choice->GetSelection()).ToStdString();

        // Instruct simulator to start cleaning
        simulator_->startCleaning(robotName);

        // Create and save "Started Cleaning" alert
        auto robot = std::make_shared<Robot>(robotName, 100); // Placeholder robot
        auto room = std::make_shared<Room>("Cleaning Area", 1);
        auto alert = std::make_shared<Alert>("Started Cleaning",
                                             "Robot " + robotName + " has started cleaning",
                                             robot, room, std::time(nullptr));
        alertSystem->sendAlert(currentUser.get(), alert);
        dbAdapter->saveAlert(*alert);
    }
}

void RobotManagementFrame::OnStopCleaning(wxCommandEvent& evt) {
    wxChoice* choice = dynamic_cast<wxChoice*>(FindWindow(wxID_ANY));
    if (choice && choice->GetSelection() != wxNOT_FOUND) {
        std::string robotName = choice->GetString(choice->GetSelection()).ToStdString();

        // Instruct simulator to stop cleaning
        simulator_->stopCleaning(robotName);

        // Create and save "Stopped Cleaning" alert
        auto robot = std::make_shared<Robot>(robotName, 100); // Placeholder robot
        auto room = std::make_shared<Room>("Cleaning Area", 1);
        auto alert = std::make_shared<Alert>("Stopped Cleaning",
                                             "Robot " + robotName + " has stopped cleaning",
                                             robot, room, std::time(nullptr));
        alertSystem->sendAlert(currentUser.get(), alert);
        dbAdapter->saveAlert(*alert);
    }
}

void RobotManagementFrame::OnReturnToCharger(wxCommandEvent& evt) {
    wxChoice* choice = dynamic_cast<wxChoice*>(FindWindow(wxID_ANY));
    if (choice && choice->GetSelection() != wxNOT_FOUND) {
        std::string robotName = choice->GetString(choice->GetSelection()).ToStdString();

        // Instruct simulator to return to charger
        simulator_->returnToCharger(robotName);

        // Create and save "Charging" alert
        auto robot = std::make_shared<Robot>(robotName, 100); // Placeholder robot
        auto room = std::make_shared<Room>("Charging Station", 0);
        auto alert = std::make_shared<Alert>("Charging",
                                             "Robot " + robotName + " has returned to charger",
                                             robot, room, std::time(nullptr));
        alertSystem->sendAlert(currentUser.get(), alert);
        dbAdapter->saveAlert(*alert);
    }
}

// Implement other methods as needed...

