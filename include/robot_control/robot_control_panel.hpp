#ifndef ROBOT_CONTROL_PANEL_HPP
#define ROBOT_CONTROL_PANEL_HPP

#include <wx/wx.h>
#include <memory>

class RobotSimulator;
class Scheduler;
class Room;

class RobotControlPanel : public wxPanel {
public:
    RobotControlPanel(wxWindow* parent, std::shared_ptr<RobotSimulator> simulator, std::shared_ptr<Scheduler> scheduler);
    ~RobotControlPanel();

private:
    void CreateControls();
    void UpdateRobotList();
    void UpdateRoomList();

    void OnRobotSelected(wxCommandEvent& event);
    void OnStartCleaning(wxCommandEvent& event);
    void OnStopCleaning(wxCommandEvent& event);
    void OnReturnToCharger(wxCommandEvent& event);
    void OnMoveToRoom(wxCommandEvent& event);
    void OnPickUpRobot(wxCommandEvent& event);

    std::shared_ptr<RobotSimulator> simulator_;
    std::shared_ptr<Scheduler> scheduler_;
    wxChoice* robotChoice_;
    wxChoice* roomChoice_;
    wxButton* moveButton_;
    wxButton* pickUpButton_;
    std::string selectedRobotName_;

    wxDECLARE_EVENT_TABLE();
};

#endif // ROBOT_CONTROL_PANEL_HPP

