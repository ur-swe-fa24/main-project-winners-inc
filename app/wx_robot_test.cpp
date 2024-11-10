// wx_robot_test.cpp

#include <mongocxx/instance.hpp>
#include <wx/wx.h>
#include "RobotManagementFrame/RobotManagementFrame.hpp"

// Main application class
class RobotApp : public wxApp {
private:
    std::unique_ptr<mongocxx::instance> mongoInstance;

public:
    bool OnInit() override {
        try {
            // Create single MongoDB instance for the application
            mongoInstance = std::make_unique<mongocxx::instance>();

            RobotManagementFrame* frame = new RobotManagementFrame();
            frame->Show();
            return true;
        } catch (const std::exception& e) {
            wxMessageBox(wxString::Format("Application initialization failed: %s", e.what()), "Error",
                         wxOK | wxICON_ERROR);
            return false;
        }
    }

    int OnExit() override {
        // Ensure MongoDB instance is cleaned up last
        mongoInstance.reset();
        return wxApp::OnExit();
    }
};

// Implement the application
wxIMPLEMENT_APP(RobotApp);
