#pragma once

#include <wx/wx.h>
#include <wx/choice.h>
#include "RobotSimulator/RobotSimulator.hpp"
#include "Room/Room.h"

class RobotControlPanel : public wxPanel {
public:
    RobotControlPanel(wxWindow* parent, RobotSimulator* simulator);
    virtual ~RobotControlPanel();  // Add virtual destructor
    
    // Event handlers - made public so they can be called from RobotManagementFrame
    void OnRobotSelected(wxCommandEvent& event);
    void OnStartCleaning(wxCommandEvent& event);
    void OnStopCleaning(wxCommandEvent& event);
    void OnReturnToCharger(wxCommandEvent& event);
    void OnMoveToRoom(wxCommandEvent& event);
    void OnPickUpRobot(wxCommandEvent& event);  // New handler for pick up
    void UpdateRobotList();  // Made public so it can be called from RobotManagementFrame

private:
    void CreateControls();
    void UpdateRoomList();
    
    // Member variables
    RobotSimulator* simulator_;
    wxChoice* robotChoice_;
    wxChoice* roomChoice_;
    wxButton* moveButton_;
    wxButton* pickUpButton_;  // New button member
    std::shared_ptr<Robot> selectedRobot_;  // Changed back to shared_ptr

    DECLARE_EVENT_TABLE()
};
