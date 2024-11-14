// LoginDialog.hpp
#ifndef LOGIN_DIALOG_HPP
#define LOGIN_DIALOG_HPP

#include <wx/wx.h>

class LoginDialog : public wxDialog {
public:
    LoginDialog(wxWindow* parent);
    std::string GetUsername() const;
    std::string GetPassword() const;

private:
    wxTextCtrl* usernameInput;
    wxTextCtrl* passwordInput;
};

#endif // LOGIN_DIALOG_HPP
