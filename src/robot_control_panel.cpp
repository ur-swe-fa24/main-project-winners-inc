#include "robot_control/robot_control_panel.hpp"
#include "RobotSimulator/RobotSimulator.hpp"
#include "map/map.h"
#include "AlertSystem/alert_system.h"
#include "Scheduler/Scheduler.hpp"
#include "CleaningTask/cleaningTask.h"
#include <wx/msgdlg.h>

wxBEGIN_EVENT_TABLE(RobotControlPanel, wxPanel)
    // event table entries
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

void RobotControlPanel::OnStartCleaning(wxCommandEvent& event) {
    if (selectedRobotName_.empty()) {
        auto alertSystem = simulator_->getAlertSystem();
        if (alertSystem) alertSystem->sendAlert("No robot selected!", "Error");
        return;
    }
    simulator_->startRobotCleaning(selectedRobotName_);
    auto alertSystem = simulator_->getAlertSystem();
    if (alertSystem) alertSystem->sendAlert("Robot started cleaning.", "Info");
}

void RobotControlPanel::OnStopCleaning(wxCommandEvent& event) {
    if (selectedRobotName_.empty()) {
        wxMessageBox("No robot selected!", "Error");
        return;
    }
    simulator_->stopRobotCleaning(selectedRobotName_);
    wxMessageBox("Robot stopped cleaning.", "Info");
}

void RobotControlPanel::OnReturnToCharger(wxCommandEvent& event) {
    if (selectedRobotName_.empty()) {
        wxMessageBox("No robot selected!", "Error");
        return;
    }
    simulator_->requestReturnToCharger(selectedRobotName_);
    wxMessageBox("Robot returning to charger.", "Info");
}


void RobotControlPanel::OnMoveToRoom(wxCommandEvent& evt) {
    if (selectedRobotName_.empty()) {
        wxMessageBox("No robot selected!", "Error", wxOK|wxICON_ERROR);
        return;
    }

    int sel = roomChoice_->GetSelection();
    if (sel == wxNOT_FOUND) {
        wxMessageBox("Please select a room", "Error", wxOK|wxICON_ERROR);
        return;
    }

    Room* targetRoom = reinterpret_cast<Room*>(roomChoice_->GetClientData(sel));
    if (!targetRoom) {
        wxMessageBox("Invalid room selection", "Error", wxOK|wxICON_ERROR);
        return;
    }

    simulator_->moveRobotToRoom(selectedRobotName_, targetRoom->getRoomId());
    wxMessageBox("Robot moving to " + targetRoom->getRoomName(), "Info");
}

void RobotControlPanel::OnPickUpRobot(wxCommandEvent& event) {
    if (selectedRobotName_.empty()) {
        wxMessageBox("No robot selected!", "Error");
        return;
    }
    simulator_->manuallyPickUpRobot(selectedRobotName_);
    wxMessageBox("Robot picked up and moved instantly to charger.", "Info");
}
