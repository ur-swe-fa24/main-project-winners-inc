// wx_robot_test.cpp

#include <mongocxx/instance.hpp>
#include <wx/wx.h>
#include <wx/cmdline.h>
#include <iostream>
#include "RobotManagementFrame/RobotManagementFrame.hpp"
#include "config/ResourceConfig.hpp"

// Main application class
class RobotApp : public wxApp {
private:
    std::unique_ptr<mongocxx::instance> mongoInstance;
    RobotManagementFrame* frame;

protected:
    void OnInitCmdLine(wxCmdLineParser& parser) override {
        parser.SetDesc(cmdLineDesc);
        parser.SetSwitchChars("-");
    }

    bool OnCmdLineParsed(wxCmdLineParser& parser) override {
        // Handle any command line arguments here
        return true;
    }

public:
    static const wxCmdLineEntryDesc cmdLineDesc[];

    bool OnInit() override {
        try {
            // Initialize command line parser
            if (!wxApp::OnInit())
                return false;

            // Set application name and vendor
            SetAppName("Robot Management System");
            SetVendorName("Winners Inc");

            // Create single MongoDB instance for the application
            mongoInstance = std::make_unique<mongocxx::instance>();

            // Initialize resource configuration
            if (!config::ResourceConfig::initialize()) {
                wxMessageBox("Failed to initialize resource configuration", "Error",
                            wxOK | wxICON_ERROR);
                return false;
            }

            // Create the main frame with title
            frame = new RobotManagementFrame("Robot Management System");
            
            // Set minimum size
            frame->SetMinSize(wxSize(1024, 768));
            
            // Center on screen
            frame->Centre();
            
            // Show the frame
            frame->Show();

            // Set as top window
            SetTopWindow(frame);

            std::cout << "Application initialized successfully" << std::endl;
            return true;
        } catch (const std::exception& e) {
            wxMessageBox(wxString::Format("Application initialization failed: %s", e.what()), 
                        "Error", wxOK | wxICON_ERROR);
            return false;
        }
    }

    int OnExit() override {
        try {
            // Clean up frame if it exists
            if (frame) {
                frame->Close(true);
            }

            // Ensure MongoDB instance is cleaned up last
            mongoInstance.reset();

            std::cout << "Application shut down successfully" << std::endl;
            return wxApp::OnExit();
        } catch (const std::exception& e) {
            std::cerr << "Error during shutdown: " << e.what() << std::endl;
            return 1;
        }
    }
};

// Define command line options
const wxCmdLineEntryDesc RobotApp::cmdLineDesc[] = {
    { wxCMD_LINE_NONE }
};

// Implement the application
wxIMPLEMENT_APP(RobotApp);
