#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// the application icon (under Windows it is in resources)
#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "key.xpm"
#endif

#include <../cryptopp/cryptlib.h>
#include <../cryptopp/aes.h>
#include <../cryptopp/modes.h>
#include <../cryptopp/filters.h>
#include <../cryptopp/pwdbased.h>
#include <../cryptopp/eax.h>
#include <../cryptopp/hex.h>
#include <../cryptopp/sha.h>
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
CryptoPP::SecByteBlock derived(32);
timer_* timer;
bool getPassword = false;
const char alphanum[] = "0123456789!@#$%^&*abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
int string_length = sizeof(alphanum) - 1;
wxIcon icon;

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

    wxImage::AddHandler( new wxPNGHandler );
	icon.LoadFile (wxT("key.png"), wxBITMAP_TYPE_PNG);

    gs_dialog = new MyDialog("Password Manager");
    gs_dialog->Show(false);
    gs_dialog->SetIcon(icon);

    return true;
}

wxBEGIN_EVENT_TABLE(MyDialog, wxDialog)
    EVT_BUTTON(wxID_HIGHEST + 1, MyDialog::OnNew)
    EVT_BUTTON(wxID_HIGHEST + 2, MyDialog::OnDelete)
    EVT_BUTTON(wxID_EXIT, MyDialog::OnExit)
    EVT_CLOSE(MyDialog::OnCloseWindow)
wxEND_EVENT_TABLE()


MyDialog::MyDialog(const wxString& title)
        : wxDialog(NULL, wxID_ANY, title)
{
    timer = new timer_();
	wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);
	wxSizerFlags flags;
	flags.Border(wxALL, 10);
	grid = new wxGrid(this, -1, wxPoint(0, 0), wxSize(400, 280));
	grid->CreateGrid(0, 2);
	grid->SetColLabelValue(0, _("Title"));
	grid->SetColLabelValue(1, _("Username"));
	//grid->SetFont?
	grid->SetMinSize(wxSize(400, 200));
	grid->SetColSize(0, 150);
	grid->SetColSize(1, 150);
	grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	grid->EnableEditing(false);
	grid->AutoSizeRows();
	grid->DisableDragRowSize();
	grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, MyDialog::OnCellClick);
	sizerTop->Add(grid, flags.Align(wxALIGN_CENTRE));
	search_input = new wxTextCtrl(this, -1, "", wxDefaultPosition);
	search_input->Bind(wxEVT_TEXT, MyDialog::OnSearch);
	sizerTop->Add(search_input, 1, wxEXPAND | wxALL, 10);
	wxSizer* const sizerBtns = new wxBoxSizer(wxHORIZONTAL);
	sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 1, wxT("&New")));
	sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 2, wxT("&Delete")));
	sizerTop->Add(sizerBtns, flags.Align(wxALIGN_CENTER_HORIZONTAL));
	SetSizerAndFit(sizerTop);
	Centre();
	m_taskBarIcon = new MyTaskBarIcon();
	if (!m_taskBarIcon->SetIcon(icon, "Password Manager")) { wxLogError(wxT("Could not set icon.")); }

#if defined(__WXOSX__) && wxOSX_USE_COCOA
    m_dockIcon = new MyTaskBarIcon(wxTBI_DOCK);
    if (!m_dockIcon->SetIcon(icon))
    {
        wxLogError("Could not set icon.");
    }
#endif

}

MyDialog::~MyDialog()
{
    delete m_taskBarIcon;
	delete timer;
}

std::string wx_to_string(wxString wx_string) {
	return std::string(wx_string.mb_str(wxConvUTF8));
}

std::string my_decrypt(std::string cipher) {
	std::string decoded;
	CryptoPP::HexDecoder decoder;
	decoder.Detach(new CryptoPP::StringSink(decoded));
	decoder.Put((CryptoPP::byte*)cipher.data(), cipher.size());
	decoder.MessageEnd();
	std::string test;
	CryptoPP::EAX<CryptoPP::AES>::Decryption decrypto;
	decrypto.SetKeyWithIV(derived.data(), 16, derived.data() + 16, 16);
	CryptoPP::AuthenticatedDecryptionFilter cf(decrypto, new CryptoPP::StringSink(test));
	cf.Put((CryptoPP::byte*)decoded.data(), decoded.size());
	cf.MessageEnd();
	return test;
}

static int callback(void* data, int argc, char** argv, char** azColName) {
	int i;
	bool found = false;
	for (i = 0; i < argc; i++) {
		//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL"); 
		if (getPassword == false) {
			if (strcmp(azColName[i], "RED") == 0) {
				grid->AppendRows(1);
				grid->SetCellValue(wxGridCellCoords(grid->GetNumberRows() - 1, 0), (wxString::FromUTF8(argv[i])));
			}
			if (strcmp(azColName[i], "YELLOW") == 0) {
				grid->SetCellValue(wxGridCellCoords(grid->GetNumberRows() - 1, 1), (wxString::FromUTF8(argv[i])));
			}
		}
		if (strcmp(azColName[i], "GREEN") == 0) {
			if (getPassword == true) {
				try {
					my_decrypt(std::string(argv[i]));
				}
				catch (const CryptoPP::Exception& exception) {
					wxMessageBox("The key entered is incorrect. Exiting.", "Error", wxOK | wxICON_EXCLAMATION);
					gs_dialog->Destroy();
					return -1;
				}
				std::string test = my_decrypt(std::string(argv[i]));
				if (wxTheClipboard->Open()) {
					gs_dialog->Show(false);
					wxTheClipboard->SetData(new wxTextDataObject(wxString::FromUTF8(test.c_str())));
					wxTheClipboard->Close();
					timer->start();
				}
			}
		}
	}
	return 0;
}

void displayData() {
	rc = sqlite3_exec(db, "SELECT * from ENTRIES order by RED;", callback, (void*)data, &zErrMsg);
	if (rc != SQLITE_OK) {
		wxMessageBox("Database error in display data func.", "Error", wxOK | wxICON_EXCLAMATION);
		sqlite3_free(zErrMsg);
	}
}

void MyDialog::OnCellClick(wxGridEvent& event) {
	std::string clicked = wx_to_string(grid->GetCellValue(wxGridCellCoords(event.GetRow(), 0)));
	getPassword = true;
	std::string stmt = "SELECT * FROM ENTRIES WHERE RED='" + clicked + "'";
	rc = sqlite3_exec(db, stmt.c_str(), callback, (void*)data, &zErrMsg);
	getPassword = false;
}

void MyDialog::OnSearch(wxCommandEvent& event) {
	std::string query = std::string(search_input->GetValue());
	if (query == "") {
		grid->DeleteRows(0, grid->GetNumberRows(), true);
		displayData();
		return;
	}
	std::string stmt = "SELECT * FROM ENTRIES WHERE RED LIKE '%" + query + "%'";
	grid->DeleteRows(0, grid->GetNumberRows(), true);
	rc = sqlite3_exec(db, stmt.c_str(), callback, (void*)data, &zErrMsg);
}

void MyDialog::OnNew(wxCommandEvent& WXUNUSED(event)) {
	wxString title = wxGetTextFromUser("Title:", "Enter new entry details", wxEmptyString);
	if (title == wxEmptyString) {
		wxMessageBox("Incomplete entry.", "Error", wxOK | wxICON_EXCLAMATION);
		return;
	}
	wxString user = wxGetTextFromUser("Username:", "Enter new entry details", wxEmptyString);
	if (user == wxEmptyString) {
		wxMessageBox("Incomplete entry.", "Error", wxOK | wxICON_EXCLAMATION);
		return;
	}
	std::string plaintext, ciphertext, recovered;
	wxMessageDialog* dial = new wxMessageDialog(NULL,
		wxT("Do you want to auto-generate password?"), wxT("Password Creation"),
		wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION);
	int res = dial->ShowModal();
	dial->Destroy();
	if (res == wxID_YES) {
		char temp[17];
		for (int i = 0; i < 16; i++) {
			temp[i] = alphanum[rand() % string_length];
		}
		temp[16] = NULL;
		plaintext = temp;
	}
	else {
		wxString pass = wxGetTextFromUser("Password:", "Enter new entry details", wxEmptyString);
		if (pass == wxEmptyString) {
			wxMessageBox("Incomplete entry.", "Error", wxOK | wxICON_EXCLAMATION);
			return;
		}
		plaintext = wx_to_string(pass);
	}
	CryptoPP::EAX<CryptoPP::AES>::Encryption encryptor;
	encryptor.SetKeyWithIV(derived.data(), 16, derived.data() + 16, 16);
	CryptoPP::AuthenticatedEncryptionFilter ef(encryptor, new CryptoPP::StringSink(ciphertext));
	ef.Put((CryptoPP::byte*)plaintext.data(), plaintext.size());
	ef.MessageEnd();
	std::string key, iv, cipher;
	CryptoPP::HexEncoder encoder;
	encoder.Detach(new CryptoPP::StringSink(cipher));
	encoder.Put((CryptoPP::byte*)ciphertext.data(), ciphertext.size());
	encoder.MessageEnd();
	std::string stmt = "INSERT INTO ENTRIES (RED,YELLOW,GREEN) VALUES ('" + wx_to_string(title) + "', '" + wx_to_string(user) + "', '" + cipher + "');";
	rc = sqlite3_exec(db, stmt.c_str(), callback, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		wxMessageBox("Database error in on new func.", "Error", wxOK | wxICON_EXCLAMATION);
		sqlite3_free(zErrMsg);
	}
	if (grid->GetNumberRows() != 0) grid->DeleteRows(0, grid->GetNumberRows(), true);
	displayData();
	//BackupData();
}

void MyDialog::OnDelete(wxCommandEvent& WXUNUSED(event)) {
	wxString title = wxGetTextFromUser("Title:", "Entry to delete", wxEmptyString);
	std::string query = wx_to_string(title);
	std::string stmt = "DELETE FROM ENTRIES WHERE RED = '" + query + "'";
	rc = sqlite3_exec(db, stmt.c_str(), callback, 0, &zErrMsg);
	if (grid->GetNumberRows() != 0) grid->DeleteRows(0, grid->GetNumberRows(), true);
	displayData();
	//BackupData();
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
    gs_dialog->Raise();
	if (master_key.compare("") == 0) {
		wxString pwd; // = wxGetPasswordFromUser("Enter key:", "Password Manager", wxEmptyString);
		wxTextEntryDialog* dlg = new wxTextEntryDialog(gs_dialog, "Password Manager", "Enter your password", "", wxOK | wxCANCEL | wxCENTRE | wxTE_PASSWORD);
		if (dlg->ShowModal() == wxID_OK)
		{
			pwd = dlg->GetValue();
		}
		else
		{
			wxMessageBox("Minimizing to taskbar", "Info", wxOK | wxICON_EXCLAMATION);
			gs_dialog->Show(false);
			//dlg->Destroy();
			return;
		}
		dlg->Destroy();
		if (!pwd.empty()) {
			unsigned int iterations = 15000;
			char purpose = 0;
			master_key = wx_to_string(pwd);
			CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> kdf;
			kdf.DeriveKey(derived.data(), derived.size(), purpose, (CryptoPP::byte*)master_key.data(), master_key.size(), NULL, 0, iterations);
			master_key = "NotAnEmptyString";
			displayData();
			search_input->SetFocus();
			return;
		}
		wxMessageBox("Password empty. Minimizing to taskbar.", "Error", wxOK | wxICON_EXCLAMATION);
		gs_dialog->Show(false);
	}
	search_input->SetFocus();
}

void MyTaskBarIcon::OnMenuExit(wxCommandEvent& )
{
    sqlite3_close(db);
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
