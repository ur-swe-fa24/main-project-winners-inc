#ifndef SCHEDULER_PANEL_HPP
#define SCHEDULER_PANEL_HPP

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/timer.h>
#include <memory>

// Forward declarations
class RobotSimulator;
class Scheduler;
class Room;

// Include cleaningTask.h here because we need CleaningTask::CleanType
#include "CleaningTask/cleaningTask.h" 

class SchedulerPanel : public wxPanel {
public:
    SchedulerPanel(wxWindow* parent, std::shared_ptr<RobotSimulator> simulator, std::shared_ptr<Scheduler> scheduler);
    ~SchedulerPanel();

    void UpdateRoomList(); 
    void UpdateRobotChoices(); 
    void UpdateTaskList();

private:
    void CreateControls();
    void BindEvents();
    void OnAssignTask(wxCommandEvent& event);
    void OnRobotSelected(wxCommandEvent& event);
    void OnRoomSelected(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);

    // Helper method to handle logic after a room is selected
    void UpdateRoomSelection(); 

    std::string cleaningStrategyToString(CleaningTask::CleanType cleanType);

    std::shared_ptr<RobotSimulator> simulator_;
    std::shared_ptr<Scheduler> scheduler_;
    wxChoice* robotChoice_;
    wxChoice* roomChoice_;
    wxChoice* strategyChoice_;
    wxListCtrl* taskListCtrl_;
    wxTimer* updateTimer_;

    wxDECLARE_EVENT_TABLE();
};

#endif // SCHEDULER_PANEL_HPP
