#ifndef SCHEDULER_PANEL_HPP
#define SCHEDULER_PANEL_HPP

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/timer.h>
#include <memory>
#include "RobotSimulator/RobotSimulator.hpp"
#include "Schedular/Schedular.hpp"
#include "Room/Room.h"
#include "map/map.h"  // Add this include for Map definition


class SchedulerPanel : public wxPanel {
public:
    SchedulerPanel(wxWindow* parent, std::shared_ptr<RobotSimulator> simulator, std::shared_ptr<Scheduler> scheduler);
    ~SchedulerPanel();

    void UpdateRoomList(); 
    void UpdateRobotChoices();

private:
    void CreateControls();
    void BindEvents();
    void OnAssignTask(wxCommandEvent& event);
    void OnRobotSelected(wxCommandEvent& event);
    void OnRoomSelected(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    void UpdateTaskList();

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
