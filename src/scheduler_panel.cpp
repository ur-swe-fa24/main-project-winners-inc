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
    if (!simulator_) return;

    // Get current selection
    int currentSelection = roomChoice_->GetSelection();
    wxString currentSelectionString;
    if (currentSelection != wxNOT_FOUND) {
        currentSelectionString = roomChoice_->GetString(currentSelection);
    }

    // Build new room list
    std::vector<std::pair<wxString, void*>> newRoomList;
    const auto& rooms = simulator_->getMap().getRooms();
    for (const auto& room : rooms) {
        if (room && !room->isRoomClean) {
            wxString roomChoiceLabel = wxString::Format("%s (%s)", room->getRoomName(), room->getSize());
            newRoomList.emplace_back(roomChoiceLabel, reinterpret_cast<void*>(room));
        }
    }

    bool needUpdate = false;
    if (newRoomList.size() != roomChoice_->GetCount()) {
        needUpdate = true;
    } else {
        for (size_t i = 0; i < newRoomList.size(); ++i) {
            if (newRoomList[i].first != roomChoice_->GetString(i)) {
                needUpdate = true;
                break;
            }
        }
    }

    if (needUpdate) {
        roomChoice_->Clear();
        for (const auto& roomEntry : newRoomList) {
            roomChoice_->Append(roomEntry.first, roomEntry.second);
        }

        // Restore previous selection if possible
        int newSelectionIndex = roomChoice_->FindString(currentSelectionString);
        if (newSelectionIndex != wxNOT_FOUND) {
            roomChoice_->SetSelection(newSelectionIndex);
        } else if (roomChoice_->GetCount() > 0) {
            roomChoice_->SetSelection(0);
        }
        roomChoice_->Refresh();
    }
}

void SchedulerPanel::UpdateRoomSelection() {
    int sel = roomChoice_->GetSelection();
    if (sel == wxNOT_FOUND) return;

    Room* selectedRoom = reinterpret_cast<Room*>(roomChoice_->GetClientData(sel));
    if (!selectedRoom) return;

    strategyChoice_->Clear();
    std::string floorType = selectedRoom->getFlooringType();
    std::transform(floorType.begin(), floorType.end(), floorType.begin(), ::tolower);

    if (floorType == "carpet") {
        strategyChoice_->Append("Vacuum");
        strategyChoice_->Append("Shampoo");
    } else if (floorType == "wood" || floorType == "tile" || floorType == "hardwood") {
        strategyChoice_->Append("Vacuum");
        strategyChoice_->Append("Scrub");
    } else {
        strategyChoice_->Append("Vacuum");
    }

    if (strategyChoice_->GetCount() > 0) {
        strategyChoice_->SetSelection(0);
    }
    strategyChoice_->Refresh();

    UpdateRobotChoices();
}



void SchedulerPanel::OnRoomSelected(wxCommandEvent& event) {
    UpdateRoomSelection();
    // After updating, force refresh to ensure UI reflects changes
    roomChoice_->Refresh();
    strategyChoice_->Refresh();
}

void SchedulerPanel::UpdateRobotChoices() {
    if (!simulator_) {
        // Log error to alert system
        auto alertSystem = simulator_->getAlertSystem();
        if (alertSystem) {
            alertSystem->sendAlert("Simulator is not initialized", "Error");
        }
        return;
    }

    if (robotChoice_) {
        wxString currentSelection;
        if (robotChoice_->GetSelection() != wxNOT_FOUND) {
            currentSelection = robotChoice_->GetString(robotChoice_->GetSelection());
        }

        robotChoice_->Clear();
        auto robotStatuses = simulator_->getRobotStatuses();
        
        for (const auto& status : robotStatuses) {
            if (!status.name.empty()) {
                robotChoice_->Append(status.name);
            }
        }

        // Try to restore the previous selection
        int index = robotChoice_->FindString(currentSelection);
        if (index != wxNOT_FOUND) {
            robotChoice_->SetSelection(index);
        } else if (robotChoice_->GetCount() > 0) {
            robotChoice_->SetSelection(0);
        }

        robotChoice_->Refresh();
    }
}

void SchedulerPanel::OnAssignTask(wxCommandEvent& event) {
    if (!robotChoice_ || robotChoice_->GetSelection() == wxNOT_FOUND) {
        auto alertSystem = simulator_->getAlertSystem();
        if (alertSystem) {
            alertSystem->sendAlert("Please select a robot.", "Error");
        }
        return;
    }
    std::string robotName = robotChoice_->GetStringSelection().ToStdString();

    if (roomChoice_->GetSelection() == wxNOT_FOUND) {
        auto alertSystem = simulator_->getAlertSystem();
        if (alertSystem) {
            alertSystem->sendAlert("Please select a room.", "Error");
        }
        return;
    }

    Room* selectedRoom = reinterpret_cast<Room*>(roomChoice_->GetClientData(roomChoice_->GetSelection()));
    if (!selectedRoom) {
        auto alertSystem = simulator_->getAlertSystem();
        if (alertSystem) {
            alertSystem->sendAlert("Invalid room selection.", "Error");
        }
        return;
    }

    if (strategyChoice_->GetSelection() == wxNOT_FOUND) {
        auto alertSystem = simulator_->getAlertSystem();
        if (alertSystem) {
            alertSystem->sendAlert("Please select a cleaning strategy.", "Error");
        }
        return;
    }
    std::string strategy = strategyChoice_->GetStringSelection().ToStdString();

    try {
        scheduler_->assignCleaningTask(robotName, selectedRoom->getRoomId(), strategy);
        // Inform user via alert panel instead of wxMessageBox
        auto alertSystem = simulator_->getAlertSystem();
        if (alertSystem) {
            alertSystem->sendAlert("Task assigned to robot " + robotName + " for room " + selectedRoom->getRoomName(), "Task");
        }
    } catch (const std::exception& e) {
        auto alertSystem = simulator_->getAlertSystem();
        if (alertSystem) {
            alertSystem->sendAlert(std::string("Assignment Error: ") + e.what(), "Error");
        }
        return;
    }

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
