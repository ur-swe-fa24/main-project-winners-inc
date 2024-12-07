#include "scheduler_panel/scheduler_panel.hpp"
#include "CleaningTask/cleaningTask.h"    // Ensure correct spelling and case
#include "Robot/Robot.h"
#include "RobotSimulator/RobotSimulator.hpp"
#include "Schedular/Schedular.hpp"
#include "Room/Room.h"
#include "AlertSystem/alert_system.h"

// Include map/map.h so that getMap() returns a fully defined type
#include "map/map.h"

#include <algorithm>
#include <iostream>

wxBEGIN_EVENT_TABLE(SchedulerPanel, wxPanel)
    // Add event table entries if needed
wxEND_EVENT_TABLE()

SchedulerPanel::SchedulerPanel(wxWindow* parent, std::shared_ptr<RobotSimulator> simulator, std::shared_ptr<Scheduler> scheduler)
    : wxPanel(parent), simulator_(simulator), scheduler_(scheduler), robotChoice_(nullptr),
      roomChoice_(nullptr), strategyChoice_(nullptr), taskListCtrl_(nullptr), updateTimer_(nullptr) {

    CreateControls();
    BindEvents();

    UpdateRoomList();
    UpdateRobotChoices();

    updateTimer_ = new wxTimer(this);
    Bind(wxEVT_TIMER, &SchedulerPanel::OnTimer, this);
    updateTimer_->Start(1000); // Update every second
}

SchedulerPanel::~SchedulerPanel() {
    if (updateTimer_) {
        updateTimer_->Stop();
        delete updateTimer_;
    }
}

void SchedulerPanel::CreateControls() {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Robot selection
    wxStaticText* robotLabel = new wxStaticText(this, wxID_ANY, "Select Robot:");
    robotChoice_ = new wxChoice(this, wxID_ANY);

    // Room selection
    wxStaticText* roomLabelStaticText = new wxStaticText(this, wxID_ANY, "Select Room:");
    roomChoice_ = new wxChoice(this, wxID_ANY);

    wxStaticText* strategyLabel = new wxStaticText(this, wxID_ANY, "Cleaning Strategy:");
    strategyChoice_ = new wxChoice(this, wxID_ANY);
    // Initially populate with some defaults; real logic in UpdateRoomSelection
    strategyChoice_->Append("Vacuum");
    strategyChoice_->Append("Scrub");
    strategyChoice_->Append("Shampoo");

    wxButton* assignTaskBtn = new wxButton(this, wxID_ANY, "Assign Task");

    sizer->Add(robotLabel, 0, wxALL, 5);
    sizer->Add(robotChoice_, 0, wxEXPAND | wxALL, 5);
    sizer->Add(roomLabelStaticText, 0, wxALL, 5);
    sizer->Add(roomChoice_, 0, wxEXPAND | wxALL, 5);
    sizer->Add(strategyLabel, 0, wxALL, 5);
    sizer->Add(strategyChoice_, 0, wxEXPAND | wxALL, 5);
    sizer->Add(assignTaskBtn, 0, wxEXPAND | wxALL, 5);

    assignTaskBtn->Bind(wxEVT_BUTTON, &SchedulerPanel::OnAssignTask, this);
    robotChoice_->Bind(wxEVT_CHOICE, &SchedulerPanel::OnRobotSelected, this);
    roomChoice_->Bind(wxEVT_CHOICE, &SchedulerPanel::OnRoomSelected, this);

    taskListCtrl_ = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    taskListCtrl_->InsertColumn(0, "Task ID");
    taskListCtrl_->InsertColumn(1, "Room");
    taskListCtrl_->InsertColumn(2, "Strategy");
    taskListCtrl_->InsertColumn(3, "Robot");
    taskListCtrl_->InsertColumn(4, "Status");

    sizer->Add(taskListCtrl_, 1, wxEXPAND | wxALL, 5);

    SetSizer(sizer);
}

void SchedulerPanel::BindEvents() {
    // Additional event bindings if needed
}

void SchedulerPanel::UpdateRoomList() {
    roomChoice_->Clear();
    // We must have included map.h so that getMap() and getRooms() are fully known
    const auto& rooms = simulator_->getMap().getRooms();
    for (auto& room : rooms) {
        if (!room->isRoomClean) {
            wxString label = wxString::Format("%s (%s)", room->getRoomName(), room->getFlooringType());
            roomChoice_->Append(label, room);
        }
    }

    if (roomChoice_->GetCount() > 0) {
        roomChoice_->SetSelection(0);
        UpdateRoomSelection(); 
    }
}

void SchedulerPanel::UpdateRoomSelection() {
    int sel = roomChoice_->GetSelection();
    if (sel == wxNOT_FOUND) return;

    Room* selectedRoom = reinterpret_cast<Room*>(roomChoice_->GetClientData(sel));
    if (!selectedRoom) return;

    strategyChoice_->Clear();
    std::string floorType = selectedRoom->getFlooringType();
    if (floorType == "Carpet") {
        strategyChoice_->Append("Vacuum");
        strategyChoice_->Append("Shampoo");
    } else if (floorType == "Wood" || floorType == "Tile") {
        strategyChoice_->Append("Vacuum");
        strategyChoice_->Append("Scrub");
    } else {
        strategyChoice_->Append("Vacuum");
    }

    if (strategyChoice_->GetCount() > 0) {
        strategyChoice_->SetSelection(0);
    }

    UpdateRobotChoices(); 
}

void SchedulerPanel::OnRoomSelected(wxCommandEvent& event) {
    UpdateRoomSelection();
}

void SchedulerPanel::UpdateRobotChoices() {
    robotChoice_->Clear();
    auto statuses = simulator_->getRobotStatuses();
    for (auto& st : statuses) {
        robotChoice_->Append(st.name);
    }
    if (robotChoice_->GetCount() > 0) {
        robotChoice_->SetSelection(0);
    }
}

void SchedulerPanel::OnAssignTask(wxCommandEvent& event) {
    if (robotChoice_->GetSelection() == wxNOT_FOUND) {
        wxMessageBox("Please select a robot.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    std::string robotName = robotChoice_->GetStringSelection().ToStdString();

    if (roomChoice_->GetSelection() == wxNOT_FOUND) {
        wxMessageBox("Please select a room.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    Room* selectedRoom = reinterpret_cast<Room*>(roomChoice_->GetClientData(roomChoice_->GetSelection()));
    if (!selectedRoom) {
        wxMessageBox("Invalid room selection.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    if (strategyChoice_->GetSelection() == wxNOT_FOUND) {
        wxMessageBox("Please select a cleaning strategy.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    std::string strategy = strategyChoice_->GetStringSelection().ToStdString();

    scheduler_->assignCleaningTask(robotName, selectedRoom->getRoomId(), strategy);

    auto alertSystem = simulator_->getAlertSystem();
    if (alertSystem) {
        alertSystem->sendAlert("Task assigned to robot " + robotName + " for room " + selectedRoom->getRoomName(), "Task");
    }

    wxMessageBox("Task assigned.", "Info");
    UpdateTaskList();
}

void SchedulerPanel::OnRobotSelected(wxCommandEvent& event) {
    // If needed, update logic based on selected robot
}

void SchedulerPanel::OnTimer(wxTimerEvent& event) {
    UpdateTaskList();
    UpdateRoomList();
    UpdateRobotChoices();
}

void SchedulerPanel::UpdateTaskList() {
    taskListCtrl_->DeleteAllItems();

    const auto& tasks = scheduler_->getAllTasks();
    int index = 0;
    for (const auto& task : tasks) {
        if (!task) continue;
        long itemIndex = taskListCtrl_->InsertItem(index, wxString::Format("%d", task->getID()));
        
        Room* room = task->getRoom();
        if (room) {
            taskListCtrl_->SetItem(itemIndex, 1, wxString::FromUTF8(room->getRoomName()));
        } else {
            taskListCtrl_->SetItem(itemIndex, 1, "Unknown Room");
        }

        taskListCtrl_->SetItem(itemIndex, 2, wxString::FromUTF8(cleaningStrategyToString(task->getCleanType())));

        if (auto robot = task->getRobot()) {
            taskListCtrl_->SetItem(itemIndex, 3, wxString::FromUTF8(robot->getName()));
        } else {
            taskListCtrl_->SetItem(itemIndex, 3, "Unassigned");
        }

        taskListCtrl_->SetItem(itemIndex, 4, wxString::FromUTF8(task->getStatus()));
        index++;
    }
}

std::string SchedulerPanel::cleaningStrategyToString(CleaningTask::CleanType cleanType) {
    switch (cleanType) {
        case CleaningTask::VACUUM: return "Vacuum";
        case CleaningTask::SCRUB: return "Scrub";
        case CleaningTask::SHAMPOO: return "Shampoo";
        default: return "Unknown";
    }
}
