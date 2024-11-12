#include "AlertSystem/alert_system.h"
#include "Robot/Robot.h"
#include "adapter/MongoDBAdapter.hpp"
#include "alert/Alert.h"
#include "role/role.h"
#include "user/user.h"
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
    LoginDialog(wxWindow *parent) : wxDialog(parent, wxID_ANY, "Login", wxDefaultPosition, wxSize(300, 200)) {

        wxPanel *panel = new wxPanel(this);
        wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

        // Username
        wxBoxSizer *hbox1 = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *usernameLabel = new wxStaticText(panel, wxID_ANY, "Username: ");
        usernameInput = new wxTextCtrl(panel, wxID_ANY);
        hbox1->Add(usernameLabel, 0, wxRIGHT, 8);
        hbox1->Add(usernameInput, 1);

        // Password
        wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *passwordLabel = new wxStaticText(panel, wxID_ANY, "Password: ");
        passwordInput = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
        hbox2->Add(passwordLabel, 0, wxRIGHT, 8);
        hbox2->Add(passwordInput, 1);

        // Buttons
        wxBoxSizer *hbox3 = new wxBoxSizer(wxHORIZONTAL);
        wxButton *loginButton = new wxButton(panel, wxID_OK, "Login");
        wxButton *cancelButton = new wxButton(panel, wxID_CANCEL, "Cancel");
        hbox3->Add(loginButton, 1);
        hbox3->Add(cancelButton, 1, wxLEFT, 5);

        vbox->Add(hbox1, 0, wxEXPAND | wxALL, 10);
        vbox->Add(hbox2, 0, wxEXPAND | wxALL, 10);
        vbox->Add(hbox3, 0, wxEXPAND | wxALL, 10);

        panel->SetSizer(vbox);
        Center();
    }

    std::string GetUsername() const { return usernameInput->GetValue().ToStdString(); }
    std::string GetPassword() const { return passwordInput->GetValue().ToStdString(); }

  private:
    wxTextCtrl *usernameInput;
    wxTextCtrl *passwordInput;
};

// Alert Dialog for displaying messages
class AlertDialog : public wxDialog {
  public:
    AlertDialog(wxWindow *parent, const std::string &message)
        : wxDialog(parent, wxID_ANY, "Alert", wxDefaultPosition, wxSize(300, 150)) {

        wxPanel *panel = new wxPanel(this);
        wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

        wxStaticText *messageText = new wxStaticText(panel, wxID_ANY, message);
        wxButton *okButton = new wxButton(panel, wxID_OK, "OK");

        vbox->Add(messageText, 1, wxALL | wxALIGN_CENTER, 10);
        vbox->Add(okButton, 0, wxALL | wxALIGN_CENTER, 10);

        panel->SetSizer(vbox);
        Center();
    }
};

class RobotManagementFrame : public wxFrame {
  public:
    RobotManagementFrame()
        : wxFrame(nullptr, wxID_ANY, "Robot Management System", wxDefaultPosition, wxSize(800, 600)) {
        try {
            // Initialize MongoDB
            std::string uri = "mongodb://localhost:27017";
            std::string dbName = "mydb101";
            dbAdapter = std::make_unique<MongoDBAdapter>(uri, dbName);

            // Initialize users and roles
            InitializeUsers();

            // Show login dialog
            if (!ShowLogin()) {
                Close();
                return;
            }

            // Initialize alert system
            alertSystem = std::make_unique<AlertSystem>();

            // Create main panel with notebook for tabs
            wxPanel *mainPanel = new wxPanel(this);
            wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

            // Create notebook for tabs
            wxNotebook *notebook = new wxNotebook(mainPanel, wxID_ANY);

            // Create panels (this initializes robotGrid, alertsList, robotChoice)
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

            // Now load existing alerts and robot statuses from MongoDB
            LoadFromDatabase();

            // Start alert checking timer (every 5 seconds)
            alertTimer = new wxTimer(this);
            alertTimer->Start(5000); // 5 seconds

            // Bind events
            Bind(wxEVT_TIMER, &RobotManagementFrame::OnCheckAlerts, this);
            Bind(wxEVT_MENU, &RobotManagementFrame::OnExit, this, wxID_EXIT);

        } catch (const std::exception &e) {
            wxMessageBox(wxString::Format("Database initialization failed: %s", e.what()), "Error",
                         wxOK | wxICON_ERROR);
            Close();
            return;
        }
    }

    void OnRefreshAlerts(wxCommandEvent &evt) { CheckAndUpdateAlerts(); }

    ~RobotManagementFrame() {

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
    }

  private:
    static mongocxx::instance *mongoInstance;
    std::vector<std::shared_ptr<Robot>> robots;
    std::vector<std::shared_ptr<User>> users;
    std::shared_ptr<User> currentUser;
    std::unique_ptr<AlertSystem> alertSystem;
    std::unique_ptr<MongoDBAdapter> dbAdapter;
    wxGrid *robotGrid;
    wxListBox *alertsList;
    wxTimer *alertTimer;
    wxChoice *robotChoice;
    std::map<std::string, std::string> userPasswords;

    // Method to load data from MongoDB
    void LoadFromDatabase() {
        // Load alerts
        auto alerts = dbAdapter->retrieveAlerts();
        for (const auto &alert : alerts) {
            alertsList->Append(alert.getMessage());
        }

        // Load robot statuses
        auto robotStatuses = dbAdapter->retrieveRobotStatuses();
        robots.clear();
        for (const auto &robot : robotStatuses) {
            robots.push_back(robot);
        }
        UpdateRobotGrid();
    }
    void InitializeUsers() {
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

        // Store passwords (in a real system, these would be hashed)
        userPasswords["admin"] = "admin123";
        userPasswords["user"] = "user123";
        userPasswords["engineer"] = "engineer123";

        // Create some demo robots if none exist in database
        auto existingRobots = dbAdapter->retrieveRobotStatuses();
        if (existingRobots.empty()) {
            robots.push_back(std::make_shared<Robot>("CleanBot-1", 100));
            robots.push_back(std::make_shared<Robot>("CleanBot-2", 85));
            robots.push_back(std::make_shared<Robot>("CleanBot-3", 92));

            // Save initial robots to database synchronously
            for (const auto &robot : robots) {
                dbAdapter->saveRobotStatusSync(robot);
            }
        } else {
            robots = existingRobots;
        }
    }

    bool ShowLogin() {
        LoginDialog dlg(this);
        if (dlg.ShowModal() == wxID_OK) {
            std::string username = dlg.GetUsername();
            std::string password = dlg.GetPassword();

            // Check credentials
            if (userPasswords.find(username) != userPasswords.end() && userPasswords[username] == password) {
                // Find user
                for (const auto &user : users) {
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

    void CreateDashboardPanel(wxNotebook *notebook) {
        wxPanel *panel = new wxPanel(notebook);
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

        // Robot status grid
        robotGrid = new wxGrid(panel, wxID_ANY);
        robotGrid->CreateGrid(robots.size(), 3);

        // Set column headers
        robotGrid->SetColLabelValue(0, "Robot Name");
        robotGrid->SetColLabelValue(1, "Battery Level");
        robotGrid->SetColLabelValue(2, "Status");

        UpdateRobotGrid();

        sizer->Add(robotGrid, 1, wxEXPAND | wxALL, 5);

        // Refresh button
        wxButton *refreshBtn = new wxButton(panel, wxID_ANY, "Refresh Status");
        refreshBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnRefreshStatus, this);
        sizer->Add(refreshBtn, 0, wxALL, 5);

        panel->SetSizer(sizer);
        notebook->AddPage(panel, "Dashboard");
    }

    void CreateAlertsPanel(wxNotebook *notebook) {
        wxPanel *panel = new wxPanel(notebook);
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

        // Alerts list
        alertsList = new wxListBox(panel, wxID_ANY);
        sizer->Add(alertsList, 1, wxEXPAND | wxALL, 5);

        // Load existing alerts from MongoDB
        auto existingAlerts = dbAdapter->retrieveAlerts();
        for (const auto &alert : existingAlerts) {
            alertsList->Append(alert.getMessage());
        }

        // Button sizer for multiple buttons
        wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

        // Clear alerts button
        wxButton *clearBtn = new wxButton(panel, wxID_ANY, "Clear Alerts");
        clearBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnClearAlerts, this);
        buttonSizer->Add(clearBtn, 1, wxALL, 5);

        // Refresh alerts button
        wxButton *refreshBtn = new wxButton(panel, wxID_ANY, "Refresh Alerts");
        refreshBtn->Bind(wxEVT_BUTTON, &RobotManagementFrame::OnRefreshAlerts, this);
        buttonSizer->Add(refreshBtn, 1, wxALL, 5);

        sizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
        panel->SetSizer(sizer);
        notebook->AddPage(panel, "Alerts");
    }

    void CreateRobotControlPanel(wxNotebook *notebook) {
        wxPanel *panel = new wxPanel(notebook);
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

        // Robot selection
        wxBoxSizer *robotSelectionSizer = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *robotLabel = new wxStaticText(panel, wxID_ANY, "Select Robot:");
        robotChoice = new wxChoice(panel, wxID_ANY);

        for (const auto &robot : robots) {
            robotChoice->Append(robot->getName());
        }

        robotSelectionSizer->Add(robotLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        robotSelectionSizer->Add(robotChoice, 1, wxALL, 5);

        // Control buttons
        wxButton *startBtn = new wxButton(panel, wxID_ANY, "Start Cleaning");
        wxButton *stopBtn = new wxButton(panel, wxID_ANY, "Stop Cleaning");
        wxButton *chargeBtn = new wxButton(panel, wxID_ANY, "Return to Charger");

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
    void CreateUserManagementPanel(wxNotebook *notebook) {
        wxPanel *panel = new wxPanel(notebook);
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

        // Users grid
        wxGrid *userGrid = new wxGrid(panel, wxID_ANY);
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

    void CreateMenuBar() {
        wxMenuBar *menuBar = new wxMenuBar();
        wxMenu *fileMenu = new wxMenu();
        fileMenu->Append(wxID_EXIT, "Exit\tAlt-X");
        menuBar->Append(fileMenu, "File");
        SetMenuBar(menuBar);
        Bind(wxEVT_MENU, &RobotManagementFrame::OnExit, this, wxID_EXIT);
    }

    void UpdateRobotGrid() {
        for (size_t i = 0; i < robots.size(); ++i) {
            robotGrid->SetCellValue(i, 0, robots[i]->getName());
            robotGrid->SetCellValue(i, 1, wxString::Format("%d%%", robots[i]->getBatteryLevel()));
            robotGrid->SetCellValue(i, 2, robots[i]->getBatteryLevel() < 20 ? "Low Battery" : "Operational");

            // Color coding for battery levels
            if (robots[i]->getBatteryLevel() < 20) {
                robotGrid->SetCellBackgroundColour(i, 1, wxColour(255, 200, 200)); // Light red

                // Create and save low battery alert
                auto room = std::make_shared<Room>("Unknown", 0);
                auto alert =
                    std::make_shared<Alert>("Low Battery",
                                            wxString::Format("Robot %s battery level critical: %d%%",
                                                             robots[i]->getName(), robots[i]->getBatteryLevel())
                                                .ToStdString(),
                                            robots[i], room, std::time(nullptr));
                alertSystem->sendAlert(currentUser.get(), alert);
                dbAdapter->saveAlert(*alert);
            } else if (robots[i]->getBatteryLevel() < 50) {
                robotGrid->SetCellBackgroundColour(i, 1, wxColour(255, 255, 200)); // Light yellow
            } else {
                robotGrid->SetCellBackgroundColour(i, 1, wxColour(200, 255, 200)); // Light green
            }
        }
        robotGrid->AutoSize();
    }
    void CheckAndUpdateAlerts() {
        // Refresh the alerts from database
        alertsList->Clear();
        auto alerts = dbAdapter->retrieveAlerts();
        for (const auto &alert : alerts) {
            alertsList->Append(alert.getMessage());
        }
    }
    // Event Handlers
    void OnCheckAlerts(wxTimerEvent &evt) { CheckAndUpdateAlerts(); }

    void OnRefreshStatus(wxCommandEvent &evt) {
        // Simulate battery depletion
        for (auto &robot : robots) {
            robot->depleteBattery(5); // Deplete by 5%
            dbAdapter->saveRobotStatusAsync(robot);
        }
        UpdateRobotGrid();
    }

    void OnClearAlerts(wxCommandEvent &evt) {
        dbAdapter->deleteAllAlerts();
        alertsList->Clear();
    }

    void OnStartCleaning(wxCommandEvent &evt) {
        if (robotChoice && robotChoice->GetSelection() != wxNOT_FOUND) {
            std::string robotName = robotChoice->GetString(robotChoice->GetSelection()).ToStdString();

            // Find selected robot
            for (auto &robot : robots) {
                if (robot->getName() == robotName) {
                    // Create and save "Started Cleaning" alert
                    auto room = std::make_shared<Room>("Cleaning Area", 1);
                    auto alert =
                        std::make_shared<Alert>("Started Cleaning", "Robot " + robotName + " has started cleaning",
                                                robot, room, std::time(nullptr));
                    alertSystem->sendAlert(currentUser.get(), alert);
                    dbAdapter->saveAlert(*alert);
                    break;
                }
            }
        } else {
            wxMessageBox("Please select a robot before starting.", "No Robot Selected", wxOK | wxICON_WARNING);
        }
    }

    void OnStopCleaning(wxCommandEvent &evt) {
        if (robotChoice && robotChoice->GetSelection() != wxNOT_FOUND) {
            std::string robotName = robotChoice->GetString(robotChoice->GetSelection()).ToStdString();

            // Find selected robot
            for (auto &robot : robots) {
                if (robot->getName() == robotName) {
                    // Create and save "Stopped Cleaning" alert
                    auto room = std::make_shared<Room>("Cleaning Area", 1);
                    auto alert =
                        std::make_shared<Alert>("Stopped Cleaning", "Robot " + robotName + " has stopped cleaning",
                                                robot, room, std::time(nullptr));
                    alertSystem->sendAlert(currentUser.get(), alert);
                    dbAdapter->saveAlert(*alert);
                    break;
                }
            }
        } else {
            wxMessageBox("Please select a robot before stopping.", "No Robot Selected", wxOK | wxICON_WARNING);
        }
    }

    void OnReturnToCharger(wxCommandEvent &evt) {
        if (robotChoice && robotChoice->GetSelection() != wxNOT_FOUND) {
            std::string robotName = robotChoice->GetString(robotChoice->GetSelection()).ToStdString();

            // Find selected robot
            for (auto &robot : robots) {
                if (robot->getName() == robotName) {
                    robot->recharge(); // Set battery to 100%
                    dbAdapter->saveRobotStatusAsync(robot);

                    // Create and save "Charging" alert
                    auto room = std::make_shared<Room>("Charging Station", 0);
                    auto alert = std::make_shared<Alert>("Charging", "Robot " + robotName + " has returned to charger",
                                                         robot, room, std::time(nullptr));
                    alertSystem->sendAlert(currentUser.get(), alert);
                    dbAdapter->saveAlert(*alert);

                    UpdateRobotGrid();
                    break;
                }
            }
        } else {
            wxMessageBox("Please select a robot to return to charger.", "No Robot Selected", wxOK | wxICON_WARNING);
        }
    }

    void OnExit(wxCommandEvent &evt) { Close(true); }
};

// Main application class
mongocxx::instance *RobotManagementFrame::mongoInstance = nullptr;

class RobotApp : public wxApp {
  private:
    std::unique_ptr<mongocxx::instance> mongoInstance;

  public:
    bool OnInit() override {
        try {
            // Create single MongoDB instance for the application
            mongoInstance = std::make_unique<mongocxx::instance>();

            RobotManagementFrame *frame = new RobotManagementFrame();
            frame->Show();
            return true;
        } catch (const std::exception &e) {
            wxMessageBox(wxString::Format("Application initialization failed: %s", e.what()), "Error",
                         wxOK | wxICON_ERROR);
            return false;
        }
    }

    int OnExit() override {
        // Ensure MongoDB instance is cleaned up last
        mongoInstance.reset();
        return wxApp::OnExit();
    }
};

wxIMPLEMENT_APP(RobotApp);