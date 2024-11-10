// LoginDialog.cpp

#include "LoginDialog/LoginDialog.hpp"

LoginDialog::LoginDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Login", wxDefaultPosition, wxSize(300, 200)) {

    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    // Username
    wxBoxSizer* hbox1 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* usernameLabel = new wxStaticText(panel, wxID_ANY, "Username: ");
    usernameInput = new wxTextCtrl(panel, wxID_ANY);
    hbox1->Add(usernameLabel, 0, wxRIGHT, 8);
    hbox1->Add(usernameInput, 1);

    // Password
    wxBoxSizer* hbox2 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* passwordLabel = new wxStaticText(panel, wxID_ANY, "Password: ");
    passwordInput = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    hbox2->Add(passwordLabel, 0, wxRIGHT, 8);
    hbox2->Add(passwordInput, 1);

    // Buttons
    wxBoxSizer* hbox3 = new wxBoxSizer(wxHORIZONTAL);
    wxButton* loginButton = new wxButton(panel, wxID_OK, "Login");
    wxButton* cancelButton = new wxButton(panel, wxID_CANCEL, "Cancel");
    hbox3->Add(loginButton, 1);
    hbox3->Add(cancelButton, 1, wxLEFT, 5);

    vbox->Add(hbox1, 0, wxEXPAND | wxALL, 10);
    vbox->Add(hbox2, 0, wxEXPAND | wxALL, 10);
    vbox->Add(hbox3, 0, wxEXPAND | wxALL, 10);

    panel->SetSizer(vbox);
    Center();
}

std::string LoginDialog::GetUsername() const {
    return usernameInput->GetValue().ToStdString();
}

std::string LoginDialog::GetPassword() const {
    return passwordInput->GetValue().ToStdString();
}
