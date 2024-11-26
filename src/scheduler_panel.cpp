#include "scheduler_panel/scheduler_panel.hpp"

SchedulerPanel::SchedulerPanel(wxWindow* parent, RobotSimulator* simulator, Scheduler* scheduler)
    : wxPanel(parent), simulator_(simulator), scheduler_(scheduler)
{
    CreateControls();
    BindEvents();
}

void SchedulerPanel::CreateControls() {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Robot selection
    wxStaticText* robotLabel = new wxStaticText(this, wxID_ANY, "Select Robot:");
    robotChoice_ = new wxChoice(this, wxID_ANY);
    UpdateRobotChoices();

    // Room selection
    wxStaticText* roomLabel = new wxStaticText(this, wxID_ANY, "Select Room:");
    roomChoice_ = new wxChoice(this, wxID_ANY);
    
    // Populate room choices
    const auto& rooms = simulator_->getMap().getRooms();
    for (const auto& room : rooms) {
        if (room) {
            wxString roomName = wxString::FromUTF8(room->getRoomName());
            roomChoice_->Append(roomName, room);  // Store Room pointer as client data
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
    sizer->Add(roomLabel, 0, wxALL, 5);
    sizer->Add(roomChoice_, 0, wxEXPAND | wxALL, 5);
    sizer->Add(strategyLabel, 0, wxALL, 5);
    sizer->Add(strategyChoice_, 0, wxEXPAND | wxALL, 5);
    sizer->Add(assignTaskBtn, 0, wxEXPAND | wxALL, 5);

    SetSizer(sizer);
}

void SchedulerPanel::BindEvents() {
    // Find the button by iterating through children
    wxWindowList& children = GetChildren();
    for (wxWindowList::iterator it = children.begin(); it != children.end(); ++it) {
        wxButton* btn = wxDynamicCast(*it, wxButton);
        if (btn && btn->GetLabel() == "Assign Task") {
            btn->Bind(wxEVT_BUTTON, &SchedulerPanel::OnAssignTask, this);
            break;
        }
    }
    roomChoice_->Bind(wxEVT_CHOICE, &SchedulerPanel::OnRoomSelected, this);
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

    scheduler_->assignCleaningTask(robotName, selectedRoom->getRoomId(), strategy);

    // Log the task assignment
    std::cout << "Task assigned: Robot " << robotName << " to clean Room " 
              << selectedRoom->getRoomName() << " with strategy " << strategy << "." << std::endl;
}

void SchedulerPanel::OnRoomSelected(wxCommandEvent& event) {
    // Handle room selection event if needed
    event.Skip();
}
