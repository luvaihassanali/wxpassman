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
    TaskBarIcon   *taskBarIcon;
    static void InitializeIconTimer();
    static void ResetIconTimer();

protected:
    void OnExit(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    void OnNew(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnRegen(wxCommandEvent& event);
    static void OnSearch(wxCommandEvent& event);
    static void OnCellClick(wxGridEvent& event);
    static void OnCellDClick(wxGridEvent& event);
    static void OnCellRightClick(wxGridEvent& event);
    wxDECLARE_EVENT_TABLE();
};

class ClipboardTimer : public wxTimer
{
public:
    ClipboardTimer() :wxTimer() {}
    void Notify() { wxTheClipboard->Clear(); }
    void start() { wxTimer::StartOnce(3000); }
};

class IconTimer : public wxTimer
{
public:
    IconTimer() :wxTimer() {}
    void Notify() { MainDialog::ResetIconTimer(); }
    void start(int millis) { wxTimer::StartOnce(millis); }
};
