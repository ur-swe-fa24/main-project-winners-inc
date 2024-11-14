// AlertDialog.cpp

#include "AlertDialog/AlertDialog.hpp"

AlertDialog::AlertDialog(wxWindow* parent, const std::string& message)
    : wxDialog(parent, wxID_ANY, "Alert", wxDefaultPosition, wxSize(300, 150)) {

    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    wxStaticText* messageText = new wxStaticText(panel, wxID_ANY, message);
    wxButton* okButton = new wxButton(panel, wxID_OK, "OK");

    vbox->Add(messageText, 1, wxALL | wxALIGN_CENTER, 10);
    vbox->Add(okButton, 0, wxALL | wxALIGN_CENTER, 10);

    panel->SetSizer(vbox);
    Center();
}
