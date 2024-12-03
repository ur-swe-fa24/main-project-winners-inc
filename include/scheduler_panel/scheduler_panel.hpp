#ifndef SCHEDULER_PANEL_HPP
#define SCHEDULER_PANEL_HPP

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/timer.h>
#include "RobotSimulator/RobotSimulator.hpp"
#include "Schedular/Schedular.hpp"

class SchedulerPanel : public wxPanel {
public:
    SchedulerPanel(wxWindow* parent, RobotSimulator* simulator, Scheduler* scheduler);
    ~SchedulerPanel();

    void UpdateRoomList(); // Method to update room list
    void UpdateRobotChoices(); // Moved to public section

private:
    void CreateControls();
    void BindEvents();
    void OnAssignTask(wxCommandEvent& event);
    void OnRobotSelected(wxCommandEvent& event);
    void OnRoomSelected(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    void UpdateTaskList();

    // Helper function
    std::string cleaningStrategyToString(CleaningTask::CleanType cleanType);

    // Member variables
    RobotSimulator* simulator_;
    Scheduler* scheduler_;
    wxChoice* robotChoice_;
    wxChoice* roomChoice_;
    wxChoice* strategyChoice_;
    wxListCtrl* taskListCtrl_;
    wxTimer* updateTimer_;
    
    DECLARE_EVENT_TABLE()
};

#endif // SCHEDULER_PANEL_HPP
