// AlertDialog.hpp

#ifndef ALERT_DIALOG_HPP
#define ALERT_DIALOG_HPP

#include <wx/wx.h>

class AlertDialog : public wxDialog {
public:
    AlertDialog(wxWindow* parent, const std::string& message);
};

#endif // ALERT_DIALOG_HPP
