#ifndef SCHEDULER_PANEL_HPP
#define SCHEDULER_PANEL_HPP

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/grid.h>
#include "RobotSimulator/RobotSimulator.hpp"
#include "Schedular/Schedular.hpp"
#include "Room/Room.h"
#include <memory>
#include <string>
#include <iostream>

class SchedulerPanel : public wxPanel {
public:
    SchedulerPanel(wxWindow* parent, RobotSimulator* simulator, Scheduler* scheduler);
    virtual ~SchedulerPanel() = default;

    void UpdateRobotChoices();

private:
    void CreateControls();
    void BindEvents();
    void OnAssignTask(wxCommandEvent& event);
    void OnRoomSelected(wxCommandEvent& event);

    RobotSimulator* simulator_;
    Scheduler* scheduler_;

    wxChoice* robotChoice_;
    wxChoice* roomChoice_;
    wxChoice* strategyChoice_;
};

#endif // SCHEDULER_PANEL_HPP
