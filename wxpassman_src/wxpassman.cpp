#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// the application icon (under Windows it is in resources)
#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "sample.xpm"
#endif

#include "wx/clipbrd.h"
#include "wx/grid.h"
#include "wx/taskbar.h"

#include "wxpassman.h"
#include "./sqlite3-3.35.2/sqlite3.h"

static MyDialog* gs_dialog = NULL;
std::string master_key = "";
wxGrid* grid;
wxTextCtrl* search_input;
sqlite3* db;
char* zErrMsg = 0;
int rc;
char* sql;
const char* data = "SQL SELECT:\n";
//CryptoPP::SecByteBlock derived(32);
timer_* timer;
bool getPassword = false;
const char alphanum[] = "0123456789!@#$%^&*abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
int string_length = sizeof(alphanum) - 1;

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

	if (!wxTaskBarIcon::IsAvailable()) {
		wxMessageBox("There appears to be no system tray support in your current environment. Exiting.", "Error", wxOK | wxICON_EXCLAMATION);
		return false;
	}

	rc = sqlite3_open_v2("./data.db", &db, SQLITE_OPEN_READWRITE, NULL);
	if (rc != SQLITE_OK) {
		wxMessageBox("Database cannot be found. Exiting.", "Error", wxOK | wxICON_EXCLAMATION);
		return false;
	}
    
    gs_dialog = new MyDialog("Password Manager");
    gs_dialog->Show(false);
    gs_dialog->SetIcon(wxICON(sample));

    return true;
}

wxBEGIN_EVENT_TABLE(MyDialog, wxDialog)
    EVT_BUTTON(wxID_EXIT, MyDialog::OnExit)
    EVT_CLOSE(MyDialog::OnCloseWindow)
wxEND_EVENT_TABLE()


MyDialog::MyDialog(const wxString& title)
        : wxDialog(NULL, wxID_ANY, title)
{
    //timer = new timer_();
	wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);
	wxSizerFlags flags;
	flags.Border(wxALL, 10);
	grid = new wxGrid(this, -1, wxPoint(0, 0), wxSize(400, 280));
	grid->CreateGrid(0, 2);
	grid->SetColLabelValue(0, _("Title"));
	grid->SetColLabelValue(1, _("Username"));
	grid->SetMinSize(wxSize(400, 200));
	grid->SetColSize(0, 150);
	grid->SetColSize(1, 150);
	grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	grid->EnableEditing(false);
	grid->AutoSizeRows();
	grid->DisableDragRowSize();
	//grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, MyDialog::OnCellClick);
	sizerTop->Add(grid, flags.Align(wxALIGN_CENTRE));
	search_input = new wxTextCtrl(this, -1, "", wxDefaultPosition);
	//search_input->Bind(wxEVT_TEXT, MyDialog::OnSearch);
	sizerTop->Add(search_input, 1, wxEXPAND | wxALL, 10);
	wxSizer* const sizerBtns = new wxBoxSizer(wxHORIZONTAL);
	sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 1, wxT("&New")));
	sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 2, wxT("&Delete")));
	sizerTop->Add(sizerBtns, flags.Align(wxALIGN_CENTER_HORIZONTAL));
	SetSizerAndFit(sizerTop);
	Centre();
	m_taskBarIcon = new MyTaskBarIcon();
	if (!m_taskBarIcon->SetIcon(wxICON(sample), "Password Manager")) { wxLogError(wxT("Could not set icon.")); }
}

MyDialog::~MyDialog()
{
    delete m_taskBarIcon;
	delete timer;
}

void MyDialog::OnExit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void MyDialog::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
    Show(false);
}

enum
{
    PU_RESTORE = 10001,
    PU_NEW_ICON,
    PU_EXIT,
    PU_CHECKMARK,
    PU_SUB1,
    PU_SUB2,
    PU_SUBMAIN
};


wxBEGIN_EVENT_TABLE(MyTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(PU_RESTORE, MyTaskBarIcon::OnMenuRestore)
    EVT_MENU(PU_EXIT,    MyTaskBarIcon::OnMenuExit)
wxEND_EVENT_TABLE()

void MyTaskBarIcon::OnMenuRestore(wxCommandEvent& )
{
    gs_dialog->Show(true);
}

void MyTaskBarIcon::OnMenuExit(wxCommandEvent& )
{
    //gs_dialog->Close(true);
	gs_dialog->Destroy();
}

// Overridables
wxMenu *MyTaskBarIcon::CreatePopupMenu()
{
    wxMenu *menu = new wxMenu;
    menu->Append(PU_RESTORE, "Show");

    /* OSX has built-in quit menu for the dock menu, but not for the status item */
#ifdef __WXOSX__
    if ( OSXIsStatusItem() )
#endif
    {
        menu->AppendSeparator();
        menu->Append(PU_EXIT,    "Exit");
    }
    return menu;
}
