//timer class to clear clipboard after 3 seconds
class timer_ : public wxTimer {
public:
    timer_():wxTimer() {}
    void Notify() { wxTheClipboard->Clear(); }
    void start() { wxTimer::StartOnce(3000); }
};

class MyTaskBarIcon : public wxTaskBarIcon {
public:
    void OnLeftButtonDClick(wxTaskBarIconEvent&);
    void OnMenuExit(wxCommandEvent&);
    virtual wxMenu *CreatePopupMenu();
    wxDECLARE_EVENT_TABLE();
};

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

class MyDialog : public wxDialog {
public:
    MyDialog(const wxString& title);
    virtual ~MyDialog();
protected:
    void OnNew(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    static void OnSearch(wxCommandEvent& event);
    static void OnCellClick(wxGridEvent& event);
    MyTaskBarIcon   *m_taskBarIcon;
    wxDECLARE_EVENT_TABLE();
};
