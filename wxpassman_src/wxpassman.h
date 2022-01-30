class timer_ : public wxTimer {
public:
    timer_() :wxTimer() {}
    void Notify() { /*wxTheClipboard->Clear();*/ }
    void start() { wxTimer::StartOnce(3000); }
};


class MyTaskBarIcon : public wxTaskBarIcon
{
public:
    MyTaskBarIcon(){}

    void OnMenuRestore(wxCommandEvent&);
    void OnMenuExit(wxCommandEvent&);
    virtual wxMenu *CreatePopupMenu() wxOVERRIDE;

    wxDECLARE_EVENT_TABLE();
};


// Define a new application
class MyApp : public wxApp
{
public:
    virtual bool OnInit() wxOVERRIDE;
};

class MyDialog: public wxDialog
{
public:
    MyDialog(const wxString& title);
    virtual ~MyDialog();

protected:
    void OnExit(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);

    MyTaskBarIcon   *m_taskBarIcon;

    wxDECLARE_EVENT_TABLE();
};
