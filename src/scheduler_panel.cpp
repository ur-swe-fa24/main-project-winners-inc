#include "scheduler_panel/scheduler_panel.hpp"
#include <algorithm>
#include <iostream>

BEGIN_EVENT_TABLE(SchedulerPanel, wxPanel)
    // Event table entries can be added here if needed
END_EVENT_TABLE()

SchedulerPanel::SchedulerPanel(wxWindow* parent, RobotSimulator* simulator, Scheduler* scheduler)
    : wxPanel(parent), simulator_(simulator), scheduler_(scheduler) {
    CreateControls();
    BindEvents();
    UpdateRobotChoices();
    
    // Create and start the update timer
    updateTimer_ = new wxTimer(this);
    this->Bind(wxEVT_TIMER, &SchedulerPanel::OnTimer, this);
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
    UpdateRobotChoices();

    // Room selection
    wxStaticText* roomLabelStaticText = new wxStaticText(this, wxID_ANY, "Select Room:");
    roomChoice_ = new wxChoice(this, wxID_ANY);

    // Populate room choices with only dirty rooms
    const auto& rooms = simulator_->getMap().getRooms();
    for (const auto& room : rooms) {
        if (room && !room->isRoomClean) {
            wxString roomChoiceLabel = wxString::Format("%s (%s)", room->getRoomName(), room->getSize());
            roomChoice_->Append(roomChoiceLabel, reinterpret_cast<void*>(room));
        }
    }

    // Cleaning strategy selection
    wxStaticText* strategyLabel = new wxStaticText(this, wxID_ANY, "Cleaning Strategy:");
    strategyChoice_ = new wxChoice(this, wxID_ANY);
    
    // Initial cleaning strategies
    strategyChoice_->Append("Vacuum");
    strategyChoice_->Append("Scrub");
    strategyChoice_->Append("Shampoo");

    // Assign task button
    wxButton* assignTaskBtn = new wxButton(this, wxID_ANY, "Assign Task");

    // Add controls to sizer
    sizer->Add(robotLabel, 0, wxALL, 5);
    sizer->Add(robotChoice_, 0, wxEXPAND | wxALL, 5);
    sizer->Add(roomLabelStaticText, 0, wxALL, 5);
    sizer->Add(roomChoice_, 0, wxEXPAND | wxALL, 5);
    sizer->Add(strategyLabel, 0, wxALL, 5);
    sizer->Add(strategyChoice_, 0, wxEXPAND | wxALL, 5);
    sizer->Add(assignTaskBtn, 0, wxEXPAND | wxALL, 5);

    // Bind assign task button
    assignTaskBtn->Bind(wxEVT_BUTTON, &SchedulerPanel::OnAssignTask, this);

    // Task List Control
    taskListCtrl_ = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    taskListCtrl_->InsertColumn(0, "Task ID");
    taskListCtrl_->InsertColumn(1, "Room");
    taskListCtrl_->InsertColumn(2, "Strategy");
    taskListCtrl_->InsertColumn(3, "Robot");
    taskListCtrl_->InsertColumn(4, "Status");

    // Set column widths
    taskListCtrl_->SetColumnWidth(0, 60);  // Task ID
    taskListCtrl_->SetColumnWidth(1, 150); // Room
    taskListCtrl_->SetColumnWidth(2, 100); // Strategy
    taskListCtrl_->SetColumnWidth(3, 150); // Robot
    taskListCtrl_->SetColumnWidth(4, 100); // Status

    // Add the task list control to the sizer
    sizer->Add(taskListCtrl_, 1, wxEXPAND | wxALL, 5);

    SetSizer(sizer);
}

void SchedulerPanel::BindEvents() {
    // Bind the room choice selection to update strategies based on room type
    roomChoice_->Bind(wxEVT_CHOICE, &SchedulerPanel::OnRoomSelected, this);
}

void SchedulerPanel::OnAssignTask(wxCommandEvent& event) {
    if (!robotChoice_ || robotChoice_->GetSelection() == wxNOT_FOUND) {
        wxMessageBox("Please select a robot.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    std::string robotName = robotChoice_->GetStringSelection().ToStdString();

    if (roomChoice_->GetSelection() == wxNOT_FOUND) {
        wxMessageBox("Please select a room.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    Room* selectedRoom = reinterpret_cast<Room*>(roomChoice_->GetClientData(roomChoice_->GetSelection()));

    if (strategyChoice_->GetSelection() == wxNOT_FOUND) {
        wxMessageBox("Please select a cleaning strategy.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    std::string strategy = strategyChoice_->GetStringSelection().ToStdString();

    // Assign the cleaning task
    try {
        scheduler_->assignCleaningTask(robotName, selectedRoom->getRoomId(), strategy);
    } catch (const std::exception& e) {
        wxMessageBox(e.what(), "Assignment Error", wxOK | wxICON_ERROR);
        return;
    }

    // Log the task assignment
    std::cout << "Task assigned: Robot " << robotName << " to clean Room "
              << selectedRoom->getRoomName() << " with strategy " << strategy << "." << std::endl;

    UpdateTaskList();
}

void SchedulerPanel::OnRobotSelected(wxCommandEvent& event) {
    // Currently not used, but can be implemented if needed
}

void SchedulerPanel::OnRoomSelected(wxCommandEvent& event) {
    if (roomChoice_->GetSelection() == wxNOT_FOUND) return;

    Room* selectedRoom = reinterpret_cast<Room*>(roomChoice_->GetClientData(roomChoice_->GetSelection()));
    if (!selectedRoom) return;

    std::string flooringType = selectedRoom->getFlooringType();

    // Convert flooringType to lowercase for case-insensitive comparison
    std::transform(flooringType.begin(), flooringType.end(), flooringType.begin(), ::tolower);

    strategyChoice_->Clear();

    if (flooringType == "carpet") {
        strategyChoice_->Append("Vacuum");
        strategyChoice_->Append("Shampoo");
    } else if (flooringType == "wood" || flooringType == "tile" || flooringType == "hardwood") {
        strategyChoice_->Append("Vacuum");
        strategyChoice_->Append("Scrub");
    } else {
        // Default strategies
        strategyChoice_->Append("Vacuum");
    }

    // Optionally, select the first strategy
    if (strategyChoice_->GetCount() > 0) {
        strategyChoice_->SetSelection(0);
    }
}

void SchedulerPanel::OnTimer(wxTimerEvent& event) {
    UpdateTaskList();
    UpdateRoomList(); // Refresh the room list to exclude clean rooms
}

void SchedulerPanel::UpdateTaskList() {
    taskListCtrl_->DeleteAllItems();

    const auto& tasks = scheduler_->getAllTasks();

    int index = 0;
    for (const auto& task : tasks) {
        if (!task) continue;

        long itemIndex = taskListCtrl_->InsertItem(index, wxString::Format("%d", task->getID()));
        
        if (Room* room = task->getRoom()) {
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

void SchedulerPanel::UpdateRobotChoices() {
    if (!simulator_) {
        wxLogError("Simulator is not initialized");
        return;
    }

    if (robotChoice_) {
        try {
            wxString currentSelection = robotChoice_->GetStringSelection();

            robotChoice_->Clear();
            auto robotStatuses = simulator_->getRobotStatuses();
            
            for (const auto& status : robotStatuses) {
                if (!status.name.empty()) {
                    robotChoice_->Append(status.name);
                }
            }

            // Restore the selection
            int index = robotChoice_->FindString(currentSelection);
            if (index != wxNOT_FOUND) {
                robotChoice_->SetSelection(index);
            } else if (robotChoice_->GetCount() > 0) {
                robotChoice_->SetSelection(0);
            }

            robotChoice_->Refresh();
        } catch (const std::exception& e) {
            wxLogError("Error updating scheduler robot choices: %s", e.what());
        } catch (...) {
            wxLogError("Unknown error occurred while updating scheduler robot choices");
        }
    }
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

    // Compare new room list with existing one
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
    }
}
