#ifndef SCHEDULER_PANEL_HPP
#define SCHEDULER_PANEL_HPP

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/listctrl.h>
#include "RobotSimulator/RobotSimulator.hpp"
#include "Schedular/Schedular.hpp"
#include "Room/Room.h"
#include "cleaningTask/cleaningTask.h"
#include <memory>
#include <string>
#include <iostream>

class SchedulerPanel : public wxPanel {
public:
    SchedulerPanel(wxWindow* parent, RobotSimulator* simulator, Scheduler* scheduler);
    virtual ~SchedulerPanel();

    void UpdateRobotChoices();
    void UpdateRoomChoices();
    void UpdateStrategyChoices();
    void UpdateTaskList();

private:
    void CreateControls();
    void BindEvents();
    void OnAssignTask(wxCommandEvent& event);
    void OnRobotSelected(wxCommandEvent& event);
    void OnRoomSelected(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    std::string cleaningStrategyToString(CleaningTask::CleanType cleanType);

    RobotSimulator* simulator_;
    Scheduler* scheduler_;

    wxChoice* robotChoice_;
    wxChoice* roomChoice_;
    wxChoice* strategyChoice_;
    wxListCtrl* taskListCtrl_;
    wxTimer* updateTimer_;

    enum {
        ID_ROBOT_CHOICE = wxID_HIGHEST + 1,
        ID_ROOM_CHOICE,
        ID_STRATEGY_CHOICE,
        ID_ASSIGN_TASK
    };
};

#endif // SCHEDULER_PANEL_HPP
