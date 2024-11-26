#include "robot_control/robot_control_panel.hpp"
#include <wx/wx.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <ctime>

BEGIN_EVENT_TABLE(RobotControlPanel, wxPanel)
END_EVENT_TABLE()

RobotControlPanel::RobotControlPanel(wxWindow* parent, RobotSimulator* simulator)
    : wxPanel(parent), simulator_(simulator), selectedRobot_(nullptr)
{
    if (!simulator_) {
        wxMessageBox("Invalid simulator pointer", "Error", wxOK | wxICON_ERROR);
        return;
    }
    CreateControls();
    UpdateRobotList();
    UpdateRoomList();
}

RobotControlPanel::~RobotControlPanel()
{
    // wxWidgets will handle deleting the child windows
}

void RobotControlPanel::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Robot selection
    wxBoxSizer* robotSelectionSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* robotLabel = new wxStaticText(this, wxID_ANY, "Select Robot:");
    robotChoice_ = new wxChoice(this, wxID_ANY);

    robotSelectionSizer->Add(robotLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    robotSelectionSizer->Add(robotChoice_, 1, wxALL, 5);

    // Room selection
    wxBoxSizer* roomSelectionSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* roomLabel = new wxStaticText(this, wxID_ANY, "Select Room:");
    roomChoice_ = new wxChoice(this, wxID_ANY);
    roomChoice_->Enable(false);  // Disabled until robot is selected

    roomSelectionSizer->Add(roomLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    roomSelectionSizer->Add(roomChoice_, 1, wxALL, 5);

    // Control buttons
    wxButton* startBtn = new wxButton(this, wxID_ANY, "Start Cleaning");
    wxButton* stopBtn = new wxButton(this, wxID_ANY, "Stop Cleaning");
    wxButton* chargeBtn = new wxButton(this, wxID_ANY, "Return to Charger");
    moveButton_ = new wxButton(this, wxID_ANY, "Move to Room");
    pickUpButton_ = new wxButton(this, wxID_ANY, "Pick Up Robot");
    moveButton_->Enable(false);  // Disabled until robot is selected
    pickUpButton_->Enable(false);  // Disabled until robot is selected

    // Bind events
    robotChoice_->Bind(wxEVT_CHOICE, &RobotControlPanel::OnRobotSelected, this);
    startBtn->Bind(wxEVT_BUTTON, &RobotControlPanel::OnStartCleaning, this);
    stopBtn->Bind(wxEVT_BUTTON, &RobotControlPanel::OnStopCleaning, this);
    chargeBtn->Bind(wxEVT_BUTTON, &RobotControlPanel::OnReturnToCharger, this);
    moveButton_->Bind(wxEVT_BUTTON, &RobotControlPanel::OnMoveToRoom, this);
    pickUpButton_->Bind(wxEVT_BUTTON, &RobotControlPanel::OnPickUpRobot, this);

    // Add to main sizer
    mainSizer->Add(robotSelectionSizer, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(roomSelectionSizer, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(startBtn, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(stopBtn, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(chargeBtn, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(moveButton_, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(pickUpButton_, 0, wxEXPAND | wxALL, 5);

    SetSizer(mainSizer);
}

void RobotControlPanel::UpdateRobotList()
{
    if (!robotChoice_ || !simulator_) {
        return;
    }

    // Store currently selected robot name if any
    wxString currentSelection;
    if (robotChoice_->GetSelection() != wxNOT_FOUND) {
        currentSelection = robotChoice_->GetString(robotChoice_->GetSelection());
    }

    robotChoice_->Clear();
    const auto& robots = simulator_->getRobots();
    int newSelectionIndex = wxNOT_FOUND;
    
    for (size_t i = 0; i < robots.size(); ++i) {
        if (robots[i]) {  // Check if robot pointer is valid
            wxString robotName = robots[i]->getName();
            robotChoice_->Append(robotName);
            
            // If this was the previously selected robot, note its new index
            if (!currentSelection.empty() && robotName == currentSelection) {
                newSelectionIndex = robotChoice_->GetCount() - 1;
            }
        }
    }

    // Restore previous selection if possible
    if (newSelectionIndex != wxNOT_FOUND) {
        robotChoice_->SetSelection(newSelectionIndex);
        // Trigger robot selected event to update UI state
        wxCommandEvent evt(wxEVT_CHOICE);
        evt.SetInt(newSelectionIndex);
        OnRobotSelected(evt);
    }
}

void RobotControlPanel::UpdateRoomList()
{
    if (!roomChoice_ || !simulator_) {
        return;
    }

    roomChoice_->Clear();
    const auto& rooms = simulator_->getMap().getRooms();
    for (const auto& room : rooms) {
        if (room) {  // Check if room pointer is valid
            wxString roomLabel = wxString::Format("%s (%s)", room->getRoomName(), room->getSize());
            roomChoice_->Append(roomLabel, reinterpret_cast<void*>(room));
        }
    }
}

void RobotControlPanel::OnRobotSelected(wxCommandEvent& event)
{
    if (!simulator_) {
        wxMessageBox("Invalid simulator pointer", "Error", wxOK | wxICON_ERROR);
        return;
    }

    int sel = robotChoice_->GetSelection();
    if (sel != wxNOT_FOUND) {
        wxString robotName = robotChoice_->GetString(sel);
        selectedRobot_ = simulator_->getRobotByName(robotName.ToStdString());
        if (selectedRobot_) {
            roomChoice_->Enable();
            moveButton_->Enable();
            pickUpButton_->Enable();
        } else {
            wxMessageBox("Failed to get robot: " + robotName, "Error", wxOK | wxICON_ERROR);
            roomChoice_->Enable(false);
            moveButton_->Enable(false);
            pickUpButton_->Enable(false);
        }
    } else {
        selectedRobot_ = nullptr;
        roomChoice_->Enable(false);
        moveButton_->Enable(false);
        pickUpButton_->Enable(false);
    }
}

void RobotControlPanel::OnStartCleaning(wxCommandEvent& evt)
{
    if (!selectedRobot_) {
        wxMessageBox("No robot selected", "Error", wxOK | wxICON_ERROR);
        return;
    }

    simulator_->startCleaning(selectedRobot_->getName());

    // Create alert
    auto alert = std::make_shared<Alert>("Robot started", 
                                       "Robot " + selectedRobot_->getName() + " has started cleaning",
                                       std::make_shared<Robot>(*selectedRobot_),
                                       nullptr,  // No specific room
                                       std::time(nullptr),
                                       Alert::LOW);

    wxMessageBox("Robot has started cleaning.", "Info", wxOK | wxICON_INFORMATION);
}

void RobotControlPanel::OnStopCleaning(wxCommandEvent& evt)
{
    if (!selectedRobot_) {
        wxMessageBox("No robot selected", "Error", wxOK | wxICON_ERROR);
        return;
    }

    simulator_->stopCleaning(selectedRobot_->getName());
    
    // Create alert
    auto alert = std::make_shared<Alert>("Robot stopped", 
                                       "Robot " + selectedRobot_->getName() + " has stopped cleaning",
                                       std::make_shared<Robot>(*selectedRobot_),
                                       nullptr,  // No specific room
                                       std::time(nullptr),
                                       Alert::LOW);

    wxMessageBox("Robot has stopped cleaning.", "Info", wxOK | wxICON_INFORMATION);
}

void RobotControlPanel::OnReturnToCharger(wxCommandEvent& evt)
{
    if (!selectedRobot_) {
        wxMessageBox("No robot selected", "Error", wxOK | wxICON_ERROR);
        return;
    }

    if (selectedRobot_->isCharging()) {
        wxMessageBox("Robot is already charging.", "Info", wxOK | wxICON_INFORMATION);
        return;
    }

    simulator_->returnToCharger(selectedRobot_->getName());

    // Create alert
    auto alert = std::make_shared<Alert>("Robot charging", 
                                       "Robot " + selectedRobot_->getName() + " is returning to charger",
                                       std::make_shared<Robot>(*selectedRobot_),
                                       nullptr,  // No specific room
                                       std::time(nullptr),
                                       Alert::LOW);

    wxMessageBox("Robot is returning to charger.", "Info", wxOK | wxICON_INFORMATION);
}

void RobotControlPanel::OnMoveToRoom(wxCommandEvent& evt)
{
    if (!simulator_) {
        wxMessageBox("Invalid simulator pointer", "Error", wxOK | wxICON_ERROR);
        return;
    }

    if (!selectedRobot_) {
        wxMessageBox("No robot selected", "Error", wxOK | wxICON_ERROR);
        return;
    }

    int sel = roomChoice_->GetSelection();
    if (sel == wxNOT_FOUND) {
        wxMessageBox("Please select a room", "Error", wxOK | wxICON_ERROR);
        return;
    }

    Room* targetRoom = reinterpret_cast<Room*>(roomChoice_->GetClientData(sel));
    if (!targetRoom) {
        wxMessageBox("Invalid room selection", "Error", wxOK | wxICON_ERROR);
        return;
    }

    Room* currentRoom = selectedRobot_->getCurrentRoom();
    if (!currentRoom) {
        wxMessageBox("Robot's current room is unknown", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // Get the path to the target room
    try {
        std::vector<int> path = simulator_->getMap().getRoute(*currentRoom, *targetRoom);
        if (path.empty()) {
            wxMessageBox("No valid path to target room found", "Error", wxOK | wxICON_ERROR);
            return;
        }

        // Set the movement path and target room
        selectedRobot_->setMovementPath(path, simulator_->getMap());
        selectedRobot_->setTargetRoom(targetRoom);

        // Create alert
        auto alert = std::make_shared<Alert>("Robot moving", 
                                           "Robot " + selectedRobot_->getName() + " is moving to " + targetRoom->getRoomName(),
                                           std::make_shared<Robot>(*selectedRobot_),
                                           std::make_shared<Room>(*targetRoom),
                                           std::time(nullptr),
                                           Alert::LOW);

        wxMessageBox(wxString::Format("Robot %s is moving to %s", 
                    selectedRobot_->getName(), 
                    targetRoom->getRoomName()), 
                    "Info", wxOK | wxICON_INFORMATION);
    } catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error moving robot: %s", e.what()), 
                    "Error", wxOK | wxICON_ERROR);
    }
}

void RobotControlPanel::OnPickUpRobot(wxCommandEvent& evt)
{
    if (!simulator_) {
        wxMessageBox("Invalid simulator pointer", "Error", wxOK | wxICON_ERROR);
        return;
    }

    if (!selectedRobot_) {
        wxMessageBox("No robot selected", "Error", wxOK | wxICON_ERROR);
        return;
    }

    try {
        // Get the charging station (assuming it's room 0)
        Room* chargingStation = simulator_->getMap().getRoomById(0);
        if (!chargingStation) {
            wxMessageBox("Charging station not found", "Error", wxOK | wxICON_ERROR);
            return;
        }

        // Create a copy of the robot for the alert
        auto robotCopy = std::make_shared<Robot>(*selectedRobot_);
        
        // Move robot to charging station and start charging
        selectedRobot_->setCurrentRoom(chargingStation);
        selectedRobot_->startCharging();

        // Create alert for robot being picked up
        auto alert = std::make_shared<Alert>("Robot picked up",
                                           "Robot " + selectedRobot_->getName() + " has been picked up and moved to charging station",
                                           robotCopy,
                                           std::make_shared<Room>(*chargingStation),
                                           std::time(nullptr),
                                           Alert::LOW);

        wxMessageBox("Robot has been picked up and moved to charging station.", 
                    "Info", wxOK | wxICON_INFORMATION);
    } catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error picking up robot: %s", e.what()), 
                    "Error", wxOK | wxICON_ERROR);
    }
}
