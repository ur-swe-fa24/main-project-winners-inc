#ifndef SCHEDULER_PANEL_HPP
#define SCHEDULER_PANEL_HPP

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/timer.h>
#include <memory>

class RobotSimulator;
class Scheduler;
class Room;
class MongoDBAdapter;
class AlertSystem;

#include "CleaningTask/cleaningTask.h"

class SchedulerPanel : public wxPanel {
public:
    SchedulerPanel(wxWindow* parent, 
                   std::shared_ptr<RobotSimulator> simulator, 
                   std::shared_ptr<Scheduler> scheduler,
                   std::shared_ptr<AlertSystem> alertSystem,
                   std::shared_ptr<MongoDBAdapter> dbAdapter);
    ~SchedulerPanel();

    void UpdateRoomList(); 
    void UpdateTaskList();

private:
    void CreateControls();
    void BindEvents();
    void OnAssignTask(wxCommandEvent& event);
    void OnRoomSelected(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    void UpdateRoomSelection(); 

    // New method to find suitable robot for given room
    std::shared_ptr<Robot> findSuitableRobotForRoom(Room* room);

    std::shared_ptr<RobotSimulator> simulator_;
    std::shared_ptr<Scheduler> scheduler_;
    std::shared_ptr<AlertSystem> alertSystem_;
    std::shared_ptr<MongoDBAdapter> dbAdapter_;

    // Removed robotChoice_ and strategyChoice_ since they are no longer user-selected
    wxChoice* roomChoice_;
    wxListCtrl* taskListCtrl_;
    wxTimer* updateTimer_;
    wxButton* assignTaskBtn_;

    wxDECLARE_EVENT_TABLE();
};

#endif // SCHEDULER_PANEL_HPP
