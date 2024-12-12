#ifndef SCHEDULER_PANEL_HPP
#define SCHEDULER_PANEL_HPP

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/timer.h>
#include <memory>
#include <vector>

class RobotSimulator;
class Scheduler;
class Room;
class MongoDBAdapter;
class AlertSystem;
class Robot;

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
    void UpdateRobotListForRoom(Room* room);

    // Method to find a single suitable robot (not used if we return multiple)
    std::shared_ptr<Robot> findSuitableRobotForRoom(Room* room);

    // Method to find all suitable robots for a given room
    std::vector<std::shared_ptr<Robot>> findSuitableRobotsForRoom(Room* room);

    std::shared_ptr<RobotSimulator> simulator_;
    std::shared_ptr<Scheduler> scheduler_;
    std::shared_ptr<AlertSystem> alertSystem_;
    std::shared_ptr<MongoDBAdapter> dbAdapter_;

    wxChoice* roomChoice_;
    wxChoice* robotChoice_; // New choice for selecting the robot
    wxListCtrl* taskListCtrl_;
    wxTimer* updateTimer_;
    wxButton* assignTaskBtn_;

    wxDECLARE_EVENT_TABLE();
};

#endif // SCHEDULER_PANEL_HPP
