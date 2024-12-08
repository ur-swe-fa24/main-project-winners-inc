#include "robot_control/robot_control_panel.hpp"
#include "RobotSimulator/RobotSimulator.hpp"
#include "Scheduler/Scheduler.hpp"
#include "Room/Room.h"
#include "Robot/Robot.h"
#include "alert/Alert.h"
#include "AlertSystem/alert_system.h"
#include "adapter/MongoDBAdapter.hpp"
#include "RobotManagementFrame/RobotManagementFrame.hpp" // For AddAlert method and frame access

#include <ctime>
#include <algorithm>
#include <memory>
#include <string>

wxBEGIN_EVENT_TABLE(RobotControlPanel, wxPanel)
// event table if needed
wxEND_EVENT_TABLE()

RobotControlPanel::RobotControlPanel(wxWindow* parent, std::shared_ptr<RobotSimulator> simulator, std::shared_ptr<Scheduler> scheduler)
    : wxPanel(parent), simulator_(simulator), scheduler_(scheduler), robotChoice_(nullptr), roomChoice_(nullptr),
      moveButton_(nullptr), pickUpButton_(nullptr) {
    CreateControls();
    UpdateRobotList();
    UpdateRoomList();
}

RobotControlPanel::~RobotControlPanel() {}

void RobotControlPanel::CreateControls() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* robotSelectionSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* robotLabel = new wxStaticText(this, wxID_ANY, "Select Robot:");
    robotChoice_ = new wxChoice(this, wxID_ANY);
    robotSelectionSizer->Add(robotLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    robotSelectionSizer->Add(robotChoice_, 1, wxALL, 5);

    wxBoxSizer* roomSelectionSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* roomLabel = new wxStaticText(this, wxID_ANY, "Select Room:");
    roomChoice_ = new wxChoice(this, wxID_ANY);
    roomChoice_->Enable(false);

    roomSelectionSizer->Add(roomLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    roomSelectionSizer->Add(roomChoice_, 1, wxALL, 5);

    wxButton* startBtn = new wxButton(this, wxID_ANY, "Start Cleaning");
    wxButton* stopBtn = new wxButton(this, wxID_ANY, "Stop Cleaning");
    wxButton* chargeBtn = new wxButton(this, wxID_ANY, "Return to Charger");
    moveButton_ = new wxButton(this, wxID_ANY, "Assign Clean Task");
    pickUpButton_ = new wxButton(this, wxID_ANY, "Pick Up Robot");
    moveButton_->Enable(false);
    pickUpButton_->Enable(false);

    robotChoice_->Bind(wxEVT_CHOICE, &RobotControlPanel::OnRobotSelected, this);
    startBtn->Bind(wxEVT_BUTTON, &RobotControlPanel::OnStartCleaning, this);
    stopBtn->Bind(wxEVT_BUTTON, &RobotControlPanel::OnStopCleaning, this);
    chargeBtn->Bind(wxEVT_BUTTON, &RobotControlPanel::OnReturnToCharger, this);
    moveButton_->Bind(wxEVT_BUTTON, &RobotControlPanel::OnMoveToRoom, this);
    pickUpButton_->Bind(wxEVT_BUTTON, &RobotControlPanel::OnPickUpRobot, this);

    mainSizer->Add(robotSelectionSizer, 0, wxEXPAND|wxALL, 5);
    mainSizer->Add(roomSelectionSizer, 0, wxEXPAND|wxALL, 5);
    mainSizer->Add(startBtn, 0, wxEXPAND|wxALL, 5);
    mainSizer->Add(stopBtn, 0, wxEXPAND|wxALL, 5);
    mainSizer->Add(chargeBtn, 0, wxEXPAND|wxALL, 5);
    mainSizer->Add(moveButton_, 0, wxEXPAND|wxALL, 5);
    mainSizer->Add(pickUpButton_, 0, wxEXPAND|wxALL, 5);

    SetSizer(mainSizer);
}

void RobotControlPanel::UpdateRobotList() {
    robotChoice_->Clear();
    auto statuses = simulator_->getRobotStatuses();
    for (auto& st : statuses) {
        robotChoice_->Append(st.name);
    }
}

void RobotControlPanel::UpdateRoomList() {
    roomChoice_->Clear();
    const auto& rooms = simulator_->getMap().getRooms();
    for (const auto& r : rooms) {
        if (!r) continue;
        wxString label = wxString::Format("%s (%s)", r->getRoomName(), r->getFlooringType());
        roomChoice_->Append(label, r);
    }
    roomChoice_->Enable(roomChoice_->GetCount() > 0);
    if (roomChoice_->GetCount() > 0) {
        roomChoice_->SetSelection(0);
    }
}

void RobotControlPanel::OnRobotSelected(wxCommandEvent& event) {
    int sel = robotChoice_->GetSelection();
    if (sel != wxNOT_FOUND) {
        selectedRobotName_ = robotChoice_->GetString(sel).ToStdString();
        roomChoice_->Enable(true);
        moveButton_->Enable(true);
        pickUpButton_->Enable(true);
    } else {
        selectedRobotName_.clear();
        roomChoice_->Enable(false);
        moveButton_->Enable(false);
        pickUpButton_->Enable(false);
    }
}

// Helper function to find a robot by name
static std::shared_ptr<Robot> findRobotByName(const std::shared_ptr<RobotSimulator>& simulator, const std::string& robotName) {
    const auto& robots = simulator->getRobots();
    for (auto& r : robots) {
        if (r->getName() == robotName) {
            return r;
        }
    }
    return nullptr;
}

// Helper: Get main frame to display alerts
RobotManagementFrame* getMainFrame(wxWindow* wnd) {
    wxWindow* top = wxGetTopLevelParent(wnd);
    return dynamic_cast<RobotManagementFrame*>(top);
}

void RobotControlPanel::OnStartCleaning(wxCommandEvent& event) {
    auto alertSystem = simulator_->getAlertSystem(); 
    auto dbAdapter = simulator_->getDbAdapter(); 
    auto frame = getMainFrame(this);

    if (selectedRobotName_.empty()) {
        if (alertSystem) alertSystem->sendAlert("No robot selected!", "Error");
        if (dbAdapter && frame) {
            Alert alert("Error", "No robot selected!", nullptr, nullptr, std::time(nullptr), Alert::HIGH);
            dbAdapter->saveAlert(alert);
            frame->AddAlert(alert);
        }
        return;
    }

    simulator_->startRobotCleaning(selectedRobotName_);
    if (alertSystem) alertSystem->sendAlert("Robot started cleaning.", "Info");
    if (dbAdapter && frame) {
        auto robot = findRobotByName(simulator_, selectedRobotName_);
        std::shared_ptr<Room> room = (robot && robot->getCurrentRoom()) ? std::make_shared<Room>(*robot->getCurrentRoom()) : nullptr;
        Alert alert("Info", "Robot started cleaning.", robot, room, std::time(nullptr), Alert::LOW);
        dbAdapter->saveAlert(alert);
        frame->AddAlert(alert);
    }
}

void RobotControlPanel::OnStopCleaning(wxCommandEvent& event) {
    auto alertSystem = simulator_->getAlertSystem();
    auto dbAdapter = simulator_->getDbAdapter();
    auto frame = getMainFrame(this);

    if (selectedRobotName_.empty()) {
        if (alertSystem) alertSystem->sendAlert("No robot selected!", "Error");
        if (dbAdapter && frame) {
            Alert alert("Error", "No robot selected!", nullptr, nullptr, std::time(nullptr), Alert::HIGH);
            dbAdapter->saveAlert(alert);
            frame->AddAlert(alert);
        }
        return;
    }

    simulator_->stopRobotCleaning(selectedRobotName_);
    if (alertSystem) alertSystem->sendAlert("Robot stopped cleaning.", "Info");
    if (dbAdapter && frame) {
        auto robot = findRobotByName(simulator_, selectedRobotName_);
        std::shared_ptr<Room> room = (robot && robot->getCurrentRoom()) ? std::make_shared<Room>(*robot->getCurrentRoom()) : nullptr;
        Alert alert("Info", "Robot stopped cleaning.", robot, room, std::time(nullptr), Alert::LOW);
        dbAdapter->saveAlert(alert);
        frame->AddAlert(alert);
    }
}

void RobotControlPanel::OnReturnToCharger(wxCommandEvent& event) {
    auto alertSystem = simulator_->getAlertSystem();
    auto dbAdapter = simulator_->getDbAdapter();
    auto frame = getMainFrame(this);

    if (selectedRobotName_.empty()) {
        if (alertSystem) alertSystem->sendAlert("No robot selected!", "Error");
        if (dbAdapter && frame) {
            Alert alert("Error", "No robot selected!", nullptr, nullptr, std::time(nullptr), Alert::HIGH);
            dbAdapter->saveAlert(alert);
            frame->AddAlert(alert);
        }
        return;
    }

    simulator_->requestReturnToCharger(selectedRobotName_);
    if (alertSystem) alertSystem->sendAlert("Robot returning to charger.", "Info");
    if (dbAdapter && frame) {
        auto robot = findRobotByName(simulator_, selectedRobotName_);
        std::shared_ptr<Room> room = (robot && robot->getCurrentRoom()) ? std::make_shared<Room>(*robot->getCurrentRoom()) : nullptr;
        Alert alert("Info", "Robot returning to charger.", robot, room, std::time(nullptr), Alert::LOW);
        dbAdapter->saveAlert(alert);
        frame->AddAlert(alert);
    }
}

void RobotControlPanel::OnMoveToRoom(wxCommandEvent& evt) {
    auto alertSystem = simulator_->getAlertSystem();
    auto dbAdapter = simulator_->getDbAdapter();
    auto frame = getMainFrame(this);

    if (selectedRobotName_.empty()) {
        if (alertSystem) alertSystem->sendAlert("No robot selected!", "Error");
        if (dbAdapter && frame) {
            Alert alert("Error", "No robot selected!", nullptr, nullptr, std::time(nullptr), Alert::HIGH);
            dbAdapter->saveAlert(alert);
            frame->AddAlert(alert);
        }
        return;
    }

    int sel = roomChoice_->GetSelection();
    if (sel == wxNOT_FOUND) {
        if (alertSystem) alertSystem->sendAlert("Please select a room.", "Error");
        if (dbAdapter && frame) {
            Alert alert("Error", "Please select a room.", nullptr, nullptr, std::time(nullptr), Alert::HIGH);
            dbAdapter->saveAlert(alert);
            frame->AddAlert(alert);
        }
        return;
    }

    Room* targetRoom = reinterpret_cast<Room*>(roomChoice_->GetClientData(sel));
    if (!targetRoom) {
        if (alertSystem) alertSystem->sendAlert("Invalid room selection.", "Error");
        if (dbAdapter && frame) {
            Alert alert("Error", "Invalid room selection.", nullptr, nullptr, std::time(nullptr), Alert::HIGH);
            dbAdapter->saveAlert(alert);
            frame->AddAlert(alert);
        }
        return;
    }

    simulator_->moveRobotToRoom(selectedRobotName_, targetRoom->getRoomId());
    if (alertSystem) alertSystem->sendAlert("Robot moving to " + targetRoom->getRoomName(), "Info");
    if (dbAdapter && frame) {
        auto robot = findRobotByName(simulator_, selectedRobotName_);
        std::shared_ptr<Room> room = std::make_shared<Room>(*targetRoom);
        Alert alert("Info", "Robot moving to " + targetRoom->getRoomName(), robot, room, std::time(nullptr), Alert::LOW);
        dbAdapter->saveAlert(alert);
        frame->AddAlert(alert);
    }
}

void RobotControlPanel::OnPickUpRobot(wxCommandEvent& event) {
    auto alertSystem = simulator_->getAlertSystem();
    auto dbAdapter = simulator_->getDbAdapter();
    auto frame = getMainFrame(this);

    if (selectedRobotName_.empty()) {
        if (alertSystem) alertSystem->sendAlert("No robot selected!", "Error");
        if (dbAdapter && frame) {
            Alert alert("Error", "No robot selected!", nullptr, nullptr, std::time(nullptr), Alert::HIGH);
            dbAdapter->saveAlert(alert);
            frame->AddAlert(alert);
        }
        return;
    }

    simulator_->manuallyPickUpRobot(selectedRobotName_);

    // Find the robot
    auto robot = findRobotByName(simulator_, selectedRobotName_);
    if (robot) {
        // Mark the robot as repaired
        robot->repair();  // We'll add this method in Robot class
    }

    if (alertSystem) alertSystem->sendAlert("Robot picked up, repaired, and moved instantly to charger.", "Info");
    if (dbAdapter && frame) {
        std::shared_ptr<Room> room = (robot && robot->getCurrentRoom()) ? std::make_shared<Room>(*robot->getCurrentRoom()) : nullptr;
        Alert alert("Info", "Robot picked up, repaired, and moved instantly to charger.", robot, room, std::time(nullptr), Alert::LOW);
        dbAdapter->saveAlert(alert);
        frame->AddAlert(alert);
    }
}
