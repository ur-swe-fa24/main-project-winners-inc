#include "robot_control/robot_control_panel.hpp"
#include "RobotManagementFrame/RobotManagementFrame.hpp"
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
        Alert alert("Error", "Invalid simulator pointer", nullptr, nullptr,
                   std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
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
        Alert alert("Error", "Invalid simulator pointer", nullptr, nullptr,
                   std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
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
            Alert alert("Error", std::string(robotName.mb_str()), nullptr, nullptr,
                       std::time(nullptr), Alert::HIGH);
            static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
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
        Alert alert("Error", "No robot selected", nullptr, nullptr,
                   std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
        return;
    }

    simulator_->startCleaning(selectedRobot_->getName());

    // Create alert
    Alert alert("Robot started", 
               "Robot " + selectedRobot_->getName() + " has started cleaning",
               selectedRobot_,
               selectedRobot_->getCurrentRoom() ? std::make_shared<Room>(*selectedRobot_->getCurrentRoom()) : nullptr,
               std::time(nullptr),
               Alert::LOW);
    
    static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
}

void RobotControlPanel::OnStopCleaning(wxCommandEvent& evt)
{
    if (!selectedRobot_) {
        Alert alert("Error", "No robot selected", nullptr, nullptr,
                   std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
        return;
    }

    simulator_->stopCleaning(selectedRobot_->getName());
    
    // Create alert
    Alert alert("Robot stopped", 
               "Robot " + selectedRobot_->getName() + " has stopped cleaning",
               selectedRobot_,
               selectedRobot_->getCurrentRoom() ? std::make_shared<Room>(*selectedRobot_->getCurrentRoom()) : nullptr,
               std::time(nullptr),
               Alert::LOW);
    
    static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
}

void RobotControlPanel::OnReturnToCharger(wxCommandEvent& evt)
{
    if (!selectedRobot_) {
        Alert alert("Error", "No robot selected", nullptr, nullptr,
                   std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
        return;
    }

    if (selectedRobot_->isCharging()) {
        Alert alert("Warning", "Robot " + selectedRobot_->getName() + " is already charging", 
                   selectedRobot_, nullptr, std::time(nullptr), Alert::LOW);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
        return;
    }

    simulator_->returnToCharger(selectedRobot_->getName());

    // Create alert
    Alert alert("Robot charging", 
               "Robot " + selectedRobot_->getName() + " is returning to charger",
               selectedRobot_,
               selectedRobot_->getCurrentRoom() ? std::make_shared<Room>(*selectedRobot_->getCurrentRoom()) : nullptr,
               std::time(nullptr),
               Alert::LOW);
    
    static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
}

void RobotControlPanel::OnMoveToRoom(wxCommandEvent& evt)
{
    if (!simulator_) {
        Alert alert("Error", "Invalid simulator pointer", nullptr, nullptr,
                   std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
        return;
    }

    if (!selectedRobot_) {
        Alert alert("Error", "No robot selected", nullptr, nullptr,
                   std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
        return;
    }

    int sel = roomChoice_->GetSelection();
    if (sel == wxNOT_FOUND) {
        Alert alert("Error", "Please select a room", selectedRobot_, nullptr,
                   std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
        return;
    }

    Room* targetRoom = reinterpret_cast<Room*>(roomChoice_->GetClientData(sel));
    if (!targetRoom) {
        Alert alert("Error", "Invalid room selection", selectedRobot_, nullptr,
                   std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
        return;
    }

    Room* currentRoom = selectedRobot_->getCurrentRoom();
    if (!currentRoom) {
        Alert alert("Error", "Robot's current room is unknown", selectedRobot_, nullptr,
                   std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
        return;
    }

    // Get the path to the target room
    try {
        std::vector<int> path = simulator_->getMap().getRoute(*currentRoom, *targetRoom);
        if (path.empty()) {
            Alert alert("Error", "No valid path to target room found", selectedRobot_, nullptr,
                       std::time(nullptr), Alert::HIGH);
            static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
            return;
        }

        // Set the movement path and target room
        selectedRobot_->setMovementPath(path, simulator_->getMap());
        selectedRobot_->setTargetRoom(targetRoom);

        // Create success alert
        Alert alert("Robot moving", 
                   "Robot " + selectedRobot_->getName() + " is moving to " + targetRoom->getRoomName(),
                   selectedRobot_,
                   std::make_shared<Room>(*targetRoom),
                   std::time(nullptr),
                   Alert::LOW);
        
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
        
    } catch (const std::exception& e) {
        Alert alert("Error", std::string("Error moving robot: ") + e.what(), 
                   selectedRobot_, nullptr, std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
    }
}

void RobotControlPanel::OnPickUpRobot(wxCommandEvent& evt)
{
    if (!simulator_) {
        Alert alert("Error", "Invalid simulator pointer", nullptr, nullptr,
                   std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
        return;
    }

    if (!selectedRobot_) {
        Alert alert("Error", "No robot selected", nullptr, nullptr,
                   std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
        return;
    }

    try {
        // Get the charging station (assuming it's room 0)
        Room* chargingStation = simulator_->getMap().getRoomById(0);
        if (!chargingStation) {
            Alert alert("Error", "Charging station not found", selectedRobot_, nullptr,
                       std::time(nullptr), Alert::HIGH);
            static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
            return;
        }

        // Set robot's location to charging station
        selectedRobot_->setCurrentRoom(chargingStation);
        selectedRobot_->setMovementPath(std::vector<int>(), simulator_->getMap()); // Clear path by setting empty vector
        selectedRobot_->setTargetRoom(nullptr);

        Alert alert("Robot picked up", 
                   "Robot " + selectedRobot_->getName() + " has been picked up and returned to charging station",
                   selectedRobot_,
                   std::make_shared<Room>(*chargingStation),
                   std::time(nullptr),
                   Alert::LOW);
        
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);

    } catch (const std::exception& e) {
        Alert alert("Error", std::string("Error picking up robot: ") + e.what(), 
                   selectedRobot_, nullptr, std::time(nullptr), Alert::HIGH);
        static_cast<RobotManagementFrame*>(wxGetTopLevelParent(this))->AddAlert(alert);
    }
}
