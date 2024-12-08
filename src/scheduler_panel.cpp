#include "scheduler_panel/scheduler_panel.hpp"
#include "CleaningTask/cleaningTask.h"
#include "Robot/Robot.h"
#include "RobotSimulator/RobotSimulator.hpp"
#include "Scheduler/Scheduler.hpp"
#include "Room/Room.h"
#include "AlertSystem/alert_system.h"
#include "adapter/MongoDBAdapter.hpp"
#include "map/map.h"

#include <wx/msgdlg.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <algorithm>
#include <iostream>

// Helper function to convert CleanType to string
static std::string cleanTypeToString(CleaningTask::CleanType ctype) {
    switch (ctype) {
        case CleaningTask::VACUUM: return "Vacuum";
        case CleaningTask::SCRUB: return "Scrub";
        case CleaningTask::SHAMPOO: return "Shampoo";
        default: return "Unknown";
    }
}

wxBEGIN_EVENT_TABLE(SchedulerPanel, wxPanel)
    // Add event table entries if needed
wxEND_EVENT_TABLE()

SchedulerPanel::SchedulerPanel(wxWindow* parent,
                               std::shared_ptr<RobotSimulator> simulator,
                               std::shared_ptr<Scheduler> scheduler,
                               std::shared_ptr<AlertSystem> alertSystem,
                               std::shared_ptr<MongoDBAdapter> dbAdapter)
    : wxPanel(parent),
      simulator_(simulator),
      scheduler_(scheduler),
      alertSystem_(alertSystem),
      dbAdapter_(dbAdapter),
      roomChoice_(nullptr),
      robotChoice_(nullptr),
      taskListCtrl_(nullptr),
      updateTimer_(nullptr)
{
    CreateControls();
    BindEvents();

    UpdateRoomList();

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

    // Room selection controls
    wxStaticText* roomLabelStaticText = new wxStaticText(this, wxID_ANY, "Select Room:");
    roomChoice_ = new wxChoice(this, wxID_ANY);

    // Robot selection controls
    wxStaticText* robotLabelStaticText = new wxStaticText(this, wxID_ANY, "Select Robot:");
    robotChoice_ = new wxChoice(this, wxID_ANY);

    assignTaskBtn_ = new wxButton(this, wxID_ANY, "Assign Task");

    sizer->Add(roomLabelStaticText, 0, wxALL, 5);
    sizer->Add(roomChoice_, 0, wxEXPAND | wxALL, 5);
    sizer->Add(robotLabelStaticText, 0, wxALL, 5);
    sizer->Add(robotChoice_, 0, wxEXPAND | wxALL, 5);
    sizer->Add(assignTaskBtn_, 0, wxEXPAND | wxALL, 5);

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
    assignTaskBtn_->Bind(wxEVT_BUTTON, &SchedulerPanel::OnAssignTask, this);
    roomChoice_->Bind(wxEVT_CHOICE, &SchedulerPanel::OnRoomSelected, this);
    // robotChoice_ doesn't need to trigger any event for now, it's just selection.
}

void SchedulerPanel::UpdateRoomList() {
    if (!simulator_) return;

    int currentSelection = roomChoice_->GetSelection();
    wxString currentSelectionString;
    if (currentSelection != wxNOT_FOUND) {
        currentSelectionString = roomChoice_->GetString(currentSelection);
    }

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

        int newSelectionIndex = roomChoice_->FindString(currentSelectionString);
        if (newSelectionIndex != wxNOT_FOUND) {
            roomChoice_->SetSelection(newSelectionIndex);
        } else if (roomChoice_->GetCount() > 0) {
            roomChoice_->SetSelection(0);
        }
        roomChoice_->Refresh();
    }

    // After updating the room list, we should also update the robot list if a room is selected
    UpdateRoomSelection();
}

void SchedulerPanel::UpdateRoomSelection() {
    int sel = roomChoice_->GetSelection();
    if (sel == wxNOT_FOUND) {
        // No room selected, clear robot list
        robotChoice_->Clear();
        robotChoice_->Enable(false);
        return;
    }

    Room* selectedRoom = reinterpret_cast<Room*>(roomChoice_->GetClientData(sel));
    if (!selectedRoom) {
        robotChoice_->Clear();
        robotChoice_->Enable(false);
        return;
    }

    // Update the robot list based on the selected room
    UpdateRobotListForRoom(selectedRoom);
}

void SchedulerPanel::UpdateRobotListForRoom(Room* room) {
    robotChoice_->Clear();
    if (!room || !simulator_) {
        robotChoice_->Enable(false);
        return;
    }

    auto suitableRobots = findSuitableRobotsForRoom(room);
    for (auto& robot : suitableRobots) {
        robotChoice_->Append(wxString::FromUTF8(robot->getName()), robot.get());
    }

    robotChoice_->Enable(robotChoice_->GetCount() > 0);
    if (robotChoice_->GetCount() > 0) {
        robotChoice_->SetSelection(0);
    }
    robotChoice_->Refresh();
}

std::shared_ptr<Robot> SchedulerPanel::findSuitableRobotForRoom(Room* room) {
    // This method is no longer needed directly since we now return a vector.
    // Keep it if you still use it elsewhere, or remove if not needed.
    auto robots = findSuitableRobotsForRoom(room);
    if (!robots.empty()) return robots.front();
    return nullptr;
}

std::vector<std::shared_ptr<Robot>> SchedulerPanel::findSuitableRobotsForRoom(Room* room) {
    std::vector<std::shared_ptr<Robot>> result;
    if (!room || !simulator_) return result;

    std::string floorType = room->getFlooringType();
    std::transform(floorType.begin(), floorType.end(), floorType.begin(), ::tolower);

    bool carpet = (floorType == "carpet");
    bool hardFloor = (floorType == "wood" || floorType == "tile" || floorType == "hardwood");

    std::vector<Robot::Strategy> acceptableStrategies;
    if (carpet) {
        acceptableStrategies = {Robot::Strategy::VACUUM, Robot::Strategy::SHAMPOO};
    } else if (hardFloor) {
        acceptableStrategies = {Robot::Strategy::VACUUM, Robot::Strategy::SCRUB};
    } else {
        acceptableStrategies = {Robot::Strategy::VACUUM};
    }

    std::string roomSize = room->getSize();
    std::transform(roomSize.begin(), roomSize.end(), roomSize.begin(), ::tolower);

    Robot::Size neededSize;
    if (roomSize == "small") {
        neededSize = Robot::Size::SMALL;
    } else if (roomSize == "medium") {
        neededSize = Robot::Size::MEDIUM;
    } else {
        neededSize = Robot::Size::LARGE;
    }

    auto& allRobots = simulator_->getRobots();
    for (auto& r : allRobots) {
        if (!r->isFailed() && r->getSize() == neededSize && 
            std::find(acceptableStrategies.begin(), acceptableStrategies.end(), r->getStrategy()) != acceptableStrategies.end()) {
            result.push_back(r);
        }
    }

    return result;
}

void SchedulerPanel::OnRoomSelected(wxCommandEvent& event) {
    UpdateRoomSelection();
    roomChoice_->Refresh();
}

void SchedulerPanel::OnAssignTask(wxCommandEvent& event) {
    if (roomChoice_->GetSelection() == wxNOT_FOUND) {
        if (alertSystem_) alertSystem_->sendAlert("Please select a room.", "Error");
        wxMessageBox("Please select a room.", "Error", wxOK|wxICON_ERROR);
        return;
    }

    if (robotChoice_->GetSelection() == wxNOT_FOUND) {
        if (alertSystem_) alertSystem_->sendAlert("Please select a robot.", "Error");
        wxMessageBox("Please select a robot.", "Error", wxOK|wxICON_ERROR);
        return;
    }

    Room* selectedRoom = reinterpret_cast<Room*>(roomChoice_->GetClientData(roomChoice_->GetSelection()));
    if (!selectedRoom) {
        if (alertSystem_) alertSystem_->sendAlert("Invalid room selection.", "Error");
        wxMessageBox("Invalid room selection.", "Error", wxOK|wxICON_ERROR);
        return;
    }

    Robot* selectedRobot = reinterpret_cast<Robot*>(robotChoice_->GetClientData(robotChoice_->GetSelection()));
    if (!selectedRobot) {
        if (alertSystem_) alertSystem_->sendAlert("Invalid robot selection.", "Error");
        wxMessageBox("Invalid robot selection.", "Error", wxOK|wxICON_ERROR);
        return;
    }

    // Determine cleaning type from the robot's strategy
    CleaningTask::CleanType ctype;
    switch (selectedRobot->getStrategy()) {
        case Robot::Strategy::VACUUM: ctype = CleaningTask::VACUUM; break;
        case Robot::Strategy::SCRUB: ctype = CleaningTask::SCRUB; break;
        case Robot::Strategy::SHAMPOO: ctype = CleaningTask::SHAMPOO; break;
    }

    try {
        scheduler_->assignCleaningTask(selectedRobot->getName(), selectedRoom->getRoomId(), cleanTypeToString(ctype));
    } catch (const std::exception& e) {
        if (alertSystem_) alertSystem_->sendAlert(std::string("Assignment Error: ") + e.what(), "Error");
        wxMessageBox(e.what(), "Assignment Error", wxOK|wxICON_ERROR);
        return;
    }

    UpdateTaskList();
}

void SchedulerPanel::OnTimer(wxTimerEvent& event) {
    UpdateTaskList();
    UpdateRoomList(); // This also updates the robot list if selection changed
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

        taskListCtrl_->SetItem(itemIndex, 2, wxString::FromUTF8(cleanTypeToString(task->getCleanType())));

        if (auto robot = task->getRobot()) {
            taskListCtrl_->SetItem(itemIndex, 3, wxString::FromUTF8(robot->getName()));
        } else {
            taskListCtrl_->SetItem(itemIndex, 3, "Unassigned");
        }

        taskListCtrl_->SetItem(itemIndex, 4, wxString::FromUTF8(task->getStatus()));
        index++;
    }
}
