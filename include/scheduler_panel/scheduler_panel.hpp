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

#include "CleaningTask/cleaningTask.h" 
#include "adapter/MongoDBAdapter.hpp"
#include "AlertSystem/alert_system.h"

class SchedulerPanel : public wxPanel {
public:
    SchedulerPanel(wxWindow* parent, 
                   std::shared_ptr<RobotSimulator> simulator, 
                   std::shared_ptr<Scheduler> scheduler,
                   std::shared_ptr<AlertSystem> alertSystem,
                   std::shared_ptr<MongoDBAdapter> dbAdapter);
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
    void UpdateRoomSelection(); 
    std::string cleaningStrategyToString(CleaningTask::CleanType cleanType);

    std::shared_ptr<RobotSimulator> simulator_;
    std::shared_ptr<Scheduler> scheduler_;
    std::shared_ptr<AlertSystem> alertSystem_;
    std::shared_ptr<MongoDBAdapter> dbAdapter_;

    wxChoice* robotChoice_;
    wxChoice* roomChoice_;
    wxChoice* strategyChoice_;
    wxListCtrl* taskListCtrl_;
    wxTimer* updateTimer_;

    wxDECLARE_EVENT_TABLE();
};

#endif // SCHEDULER_PANEL_HPP
