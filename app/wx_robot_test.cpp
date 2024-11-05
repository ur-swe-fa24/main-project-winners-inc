#include "Robot/Robot.h"
#include "alert/Alert.h"
#include "role/role.h"
#include "user/user.h"
#include <ctime>
#include <memory>
#include <vector>
#include <wx/grid.h>
#include <wx/notebook.h>
#include <wx/stattext.h>
#include <wx/wx.h>

// Login Dialog
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

class RobotManagementFrame : public wxFrame {
  public:
    RobotManagementFrame()
        : wxFrame(nullptr, wxID_ANY, "Robot Management System", wxDefaultPosition, wxSize(800, 600)) {

        // Initialize users and roles
        InitializeUsers();

        // Show login dialog
        if (!ShowLogin()) {
            Close();
            return;
        }

        // Create main panel with notebook for tabs
        wxPanel *mainPanel = new wxPanel(this);
        wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

        // Create notebook for tabs
        wxNotebook *notebook = new wxNotebook(mainPanel, wxID_ANY);

        // Dashboard Panel
        CreateDashboardPanel(notebook);

        // Robot Control Panel
        CreateRobotControlPanel(notebook);

        // Alerts Panel
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
    }

  private:
    std::vector<std::shared_ptr<Robot>> robots;
    std::vector<std::shared_ptr<User>> users;
    std::shared_ptr<User> currentUser;
    wxGrid *robotGrid;

    void InitializeUsers() {
        // Create roles
        Role adminRole("Admin");
        Role userRole("User");

        // Add permissions
        Permission adminPerm("ADMIN");
        Permission userPerm("USER");
        adminRole.addPermission(adminPerm);
        userRole.addPermission(userPerm);

        // Create users
        users.push_back(std::make_shared<User>(1, "admin", adminRole));
        users.push_back(std::make_shared<User>(2, "user", userRole));

        // Create some demo robots
        robots.push_back(std::make_shared<Robot>("CleanBot-1", 100));
        robots.push_back(std::make_shared<Robot>("CleanBot-2", 85));
        robots.push_back(std::make_shared<Robot>("CleanBot-3", 92));
    }

    bool ShowLogin() {
        LoginDialog dlg(this);
        if (dlg.ShowModal() == wxID_OK) {
            std::string username = dlg.GetUsername();
            // In the real application, we will hash the password and check against the mongodb

            // Find user
            for (const auto &user : users) {
                if (user->getName() == username) {
                    currentUser = user;
                    return true;
                }
            }
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

    void CreateRobotControlPanel(wxNotebook *notebook) {
        wxPanel *panel = new wxPanel(notebook);
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

        // Robot selection
        wxBoxSizer *robotSelectionSizer = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *robotLabel = new wxStaticText(panel, wxID_ANY, "Select Robot:");
        wxChoice *robotChoice = new wxChoice(panel, wxID_ANY);

        for (const auto &robot : robots) {
            robotChoice->Append(robot->getName());
        }

        robotSelectionSizer->Add(robotLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        robotSelectionSizer->Add(robotChoice, 1, wxALL, 5);

        // Control buttons
        wxButton *startBtn = new wxButton(panel, wxID_ANY, "Start Cleaning");
        wxButton *stopBtn = new wxButton(panel, wxID_ANY, "Stop Cleaning");
        wxButton *chargeBtn = new wxButton(panel, wxID_ANY, "Return to Charger");

        sizer->Add(robotSelectionSizer, 0, wxEXPAND | wxALL, 5);
        sizer->Add(startBtn, 0, wxEXPAND | wxALL, 5);
        sizer->Add(stopBtn, 0, wxEXPAND | wxALL, 5);
        sizer->Add(chargeBtn, 0, wxEXPAND | wxALL, 5);

        panel->SetSizer(sizer);
        notebook->AddPage(panel, "Robot Control");
    }

    void CreateAlertsPanel(wxNotebook *notebook) {
        wxPanel *panel = new wxPanel(notebook);
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

        // Alerts list
        wxListBox *alertsList = new wxListBox(panel, wxID_ANY);
        sizer->Add(alertsList, 1, wxEXPAND | wxALL, 5);

        // Clear alerts button
        wxButton *clearBtn = new wxButton(panel, wxID_ANY, "Clear Alerts");
        sizer->Add(clearBtn, 0, wxALL, 5);

        panel->SetSizer(sizer);
        notebook->AddPage(panel, "Alerts");
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

        // Add user button
        wxButton *addUserBtn = new wxButton(panel, wxID_ANY, "Add User");
        sizer->Add(addUserBtn, 0, wxALL, 5);

        panel->SetSizer(sizer);
        notebook->AddPage(panel, "User Management");
    }

    void CreateMenuBar() {
        wxMenuBar *menuBar = new wxMenuBar();

        // File menu
        wxMenu *fileMenu = new wxMenu();
        fileMenu->Append(wxID_EXIT, "Exit\tAlt-X");

        // Help menu
        wxMenu *helpMenu = new wxMenu();
        helpMenu->Append(wxID_ABOUT, "About");

        menuBar->Append(fileMenu, "File");
        menuBar->Append(helpMenu, "Help");

        SetMenuBar(menuBar);

        // Bind menu events
        Bind(wxEVT_MENU, &RobotManagementFrame::OnExit, this, wxID_EXIT);
        Bind(wxEVT_MENU, &RobotManagementFrame::OnAbout, this, wxID_ABOUT);
    }

    void UpdateRobotGrid() {
        for (size_t i = 0; i < robots.size(); ++i) {
            robotGrid->SetCellValue(i, 0, robots[i]->getName());
            robotGrid->SetCellValue(i, 1, wxString::Format("%d%%", robots[i]->getBatteryLevel()));
            robotGrid->SetCellValue(i, 2, robots[i]->getBatteryLevel() < 20 ? "Low Battery" : "Operational");
        }
        robotGrid->AutoSize();
    }

    void OnRefreshStatus(wxCommandEvent &evt) { UpdateRobotGrid(); }

    void OnExit(wxCommandEvent &evt) { Close(true); }

    void OnAbout(wxCommandEvent &evt) {
        wxMessageBox("Robot Management System\nVersion 1.0", "About", wxOK | wxICON_INFORMATION);
    }
};

class RobotApp : public wxApp {
  public:
    bool OnInit() {
        RobotManagementFrame *frame = new RobotManagementFrame();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(RobotApp);