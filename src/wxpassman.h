class timer_ : public wxTimer
{
public:
    timer_() :wxTimer() {}
    void Notify() { wxTheClipboard->Clear(); }
    void start() { wxTimer::StartOnce(3000); }
};

class TaskBarIcon : public wxTaskBarIcon
{
public:
    TaskBarIcon() {}
    void OnMenuRestore(wxCommandEvent&);
    void OnMenuExit(wxCommandEvent&);
    virtual wxMenu *CreatePopupMenu() wxOVERRIDE;
    wxDECLARE_EVENT_TABLE();
};

class wxPassman : public wxApp
{
public:
    virtual bool OnInit() wxOVERRIDE;
};

class MainDialog: public wxDialog
{
public:
    MainDialog(const wxString& title);
    virtual ~MainDialog();

protected:
    void OnExit(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    void OnNew(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);    
    static void OnSearch(wxCommandEvent& event);
    static void OnCellClick(wxGridEvent& event);
    TaskBarIcon   *taskBarIcon;
    wxDECLARE_EVENT_TABLE();
};
