#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
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
#include <wx/font.h>
#include "wx/grid.h"
#include "wx/taskbar.h"
#include "wxpassman.h"
#include "./sqlite3-3.35.2/sqlite3.h"
//#include "./ObjCFunc/ObjCCall.h"
#include "./MyObject/MyObject-C-Interface.h"

const char alphanum[] = "0123456789!@#$%^&*abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int alphanumLength = sizeof(alphanum) - 1;
const char *sqlData = "SQL SELECT:\n";

bool decryptPassword = false;
bool verifyPassword = false;
char *sqlErrMsg = 0;
CryptoPP::SecByteBlock derived(32);
MainDialog *mainDialog = NULL;
int sqlReturnCode;
sqlite3 *db;
std::string masterKey = "";
ClipboardTimer *clipboardTimer;
IconTimer *iconTimer;
wxGrid *grid;
wxIcon icon;
wxTextCtrl *searchInput;

wxIMPLEMENT_APP(wxPassman);

bool wxPassman::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	if (!wxTaskBarIcon::IsAvailable())
	{
		wxMessageBox("There appears to be no system tray support in your current environment. Exiting.", "Error", wxOK | wxICON_EXCLAMATION);
		return false;
	}

	sqlReturnCode = sqlite3_open_v2("/Volumes/HDD/data.db", &db, SQLITE_OPEN_READWRITE, NULL);
	if (sqlReturnCode != SQLITE_OK)
	{
		wxMessageBox("Database cannot be found. Exiting.", "Error", wxOK | wxICON_EXCLAMATION);
		return false;
	}

	wxImage::AddHandler(new wxPNGHandler);
	icon.LoadFile(wxT("key-b.png"), wxBITMAP_TYPE_PNG);
	mainDialog = new MainDialog("Password Manager");
	mainDialog->SetIcon(icon);
	mainDialog->Show(false);

	/*time_t currTime = time(0);
	struct tm tm_currTime;
	localtime_r(&currTime, &tm_currTime);
	mktime(&tm_currTime);

	struct tm tm_midnightTime;
	localtime_r(&currTime, &tm_midnightTime);
	tm_midnightTime.tm_sec = 0;
	tm_midnightTime.tm_min = 59;
	tm_midnightTime.tm_hour = 23;
	mktime(&tm_midnightTime);

	time_t midnightTime = mktime(&tm_midnightTime);
	double diffSecs = difftime(midnightTime, currTime);*/

	//std::string msg = std::to_string(tm_currTime.tm_year + 1900) + " " + std::to_string(tm_currTime.tm_mon + 1) + " " + std::to_string(tm_currTime.tm_mday) + " " + std::to_string(tm_currTime.tm_hour) + " " + std::to_string(tm_currTime.tm_min) + " " + std::to_string(tm_currTime.tm_sec);
	//std::string msg2 = std::to_string(tm_midnightTime.tm_year + 1900) + " " + std::to_string(tm_midnightTime.tm_mon + 1) + " " + std::to_string(tm_midnightTime.tm_mday) + " " + std::to_string(tm_midnightTime.tm_hour) + " " + std::to_string(tm_midnightTime.tm_min) + " " + std::to_string(tm_midnightTime.tm_sec);

	//wxMessageBox(std::to_string(diffSecs), "Alert", wxOK | wxICON_EXCLAMATION);

	//iconTimer = new IconTimer();
	//iconTimer->start(10000);

    //char *p = (char *) malloc(12);
    //p = 0; // the leak is here

	return true;
}

wxBEGIN_EVENT_TABLE(MainDialog, wxDialog)
EVT_BUTTON(wxID_HIGHEST + 1, MainDialog::OnNew)
EVT_BUTTON(wxID_HIGHEST + 2, MainDialog::OnDelete)
EVT_BUTTON(wxID_EXIT, MainDialog::OnExit)
EVT_CLOSE(MainDialog::OnCloseWindow)
wxEND_EVENT_TABLE()

MainDialog::MainDialog(const wxString &title) : wxDialog(NULL, wxID_ANY, title, wxPoint(10, 735)), _impl ( NULL )
{
	_impl = new MyClassImpl();
	clipboardTimer = new ClipboardTimer();
	wxSizer *const sizerTop = new wxBoxSizer(wxVERTICAL);
	wxSizerFlags flags;
	flags.Border(wxALL, 10);
	grid = new wxGrid(this, -1, wxPoint(0, 0), wxSize(400, 280));
	grid->CreateGrid(0, 2);
	grid->SetColLabelValue(0, _("Title"));
	grid->SetColLabelValue(1, _("Username"));
	wxFont f(wxFontInfo(12).FaceName("Courier"));
	grid->SetLabelFont(f);
	grid->SetMinSize(wxSize(400, 200));
	grid->SetColSize(0, 150);
	grid->SetColSize(1, 150);
	grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	grid->EnableEditing(false);
	grid->AutoSizeRows();
	grid->DisableDragRowSize();
	grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, MainDialog::OnCellClick);
	sizerTop->Add(grid, flags.Align(wxALIGN_CENTRE));
	searchInput = new wxTextCtrl(this, -1, "", wxDefaultPosition);
	searchInput->Bind(wxEVT_TEXT, MainDialog::OnSearch);
	sizerTop->Add(searchInput, 1, wxEXPAND | wxALL, 10);
	wxSizer *const sizerBtns = new wxBoxSizer(wxHORIZONTAL);
	sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 1, wxT("&New")));
	sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 2, wxT("&Delete")));
	sizerTop->Add(sizerBtns, flags.Align(wxALIGN_CENTER_HORIZONTAL));
	SetSizerAndFit(sizerTop);
	taskBarIcon = new TaskBarIcon();
	if (!taskBarIcon->SetIcon(icon, "Password Manager"))
	{
		wxLogError(wxT("Could not set icon."));
	}
}

MainDialog::~MainDialog()
{
	if ( _impl ) { delete _impl; _impl = NULL; }
	delete taskBarIcon;
	delete clipboardTimer;
	delete iconTimer;
}

void MainDialog::ChangeIcon()
{
	wxMessageBox("Check", "Alert", wxOK | wxICON_EXCLAMATION);
}

std::string WxToString(wxString wx_string)
{
	return std::string(wx_string.mb_str(wxConvUTF8));
}

std::string Decrypt(std::string cipher)
{
	std::string decoded;
	CryptoPP::HexDecoder decoder;
	decoder.Detach(new CryptoPP::StringSink(decoded));
	decoder.Put((CryptoPP::byte *)cipher.data(), cipher.size());
	decoder.MessageEnd();
	std::string secret;
	CryptoPP::EAX<CryptoPP::AES>::Decryption decrypto;
	decrypto.SetKeyWithIV(derived.data(), 16, derived.data() + 16, 16);
	CryptoPP::AuthenticatedDecryptionFilter cf(decrypto, new CryptoPP::StringSink(secret));
	cf.Put((CryptoPP::byte *)decoded.data(), decoded.size());
	cf.MessageEnd();
	return secret;
}

static int SqlExecCallback(void *data, int argc, char **argv, char **azColName)
{
	int i;
	bool found = false;
	for (i = 0; i < argc; i++)
	{
		// printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		if (decryptPassword == false)
		{
			if (strcmp(azColName[i], "RED") == 0)
			{
				grid->AppendRows(1);
				grid->SetCellValue(wxGridCellCoords(grid->GetNumberRows() - 1, 0), (wxString::FromUTF8(argv[i])));
			}
			if (strcmp(azColName[i], "YELLOW") == 0)
			{
				grid->SetCellValue(wxGridCellCoords(grid->GetNumberRows() - 1, 1), (wxString::FromUTF8(argv[i])));
			}
		}
		if (strcmp(azColName[i], "GREEN") == 0)
		{
			if (decryptPassword == true)
			{
				if (verifyPassword)
				{
					try
					{
						Decrypt(std::string(argv[i]));
					}
					catch (const CryptoPP::Exception &exception)
					{
						wxMessageBox("The key entered is incorrect. Exiting.", "Error", wxOK | wxICON_EXCLAMATION);
						mainDialog->Destroy();
						return -1;
					}
					return 0;
				}

				std::string secret = Decrypt(std::string(argv[i]));
				if (wxTheClipboard->Open())
				{
					mainDialog->Show(false);
					wxTheClipboard->SetData(new wxTextDataObject(wxString::FromUTF8(secret.c_str())));
					wxTheClipboard->Close();
					clipboardTimer->start();
				}
			}
		}
	}
	return 0;
}

void DisplayData()
{
	sqlReturnCode = sqlite3_exec(db, "SELECT * from ENTRIES order by RED;", SqlExecCallback, (void *)sqlData, &sqlErrMsg);
	if (sqlReturnCode != SQLITE_OK)
	{
		wxMessageBox("Database error in display data func.", "Error", wxOK | wxICON_EXCLAMATION);
		sqlite3_free(sqlErrMsg);
	}
}

void VerifyKey()
{
	if (grid->GetNumberRows() != 0)
	{
		std::string clicked = WxToString(grid->GetCellValue(wxGridCellCoords(0, 0)));
		decryptPassword = true;
		verifyPassword = true;
		std::string stmt = "SELECT * FROM ENTRIES WHERE RED='" + clicked + "'";
		sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, (void *)sqlData, &sqlErrMsg);
		decryptPassword = false;
		verifyPassword = false;
	}
}

void MainDialog::OnCellClick(wxGridEvent &event)
{
	std::string clicked = WxToString(grid->GetCellValue(wxGridCellCoords(event.GetRow(), 0)));
	decryptPassword = true;
	std::string stmt = "SELECT * FROM ENTRIES WHERE RED='" + clicked + "'";
	sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, (void *)sqlData, &sqlErrMsg);
	decryptPassword = false;
}

void MainDialog::OnSearch(wxCommandEvent &event)
{
	std::string query = std::string(searchInput->GetValue());
	if (query == "")
	{
		grid->DeleteRows(0, grid->GetNumberRows(), true);
		DisplayData();
		return;
	}
	std::string stmt = "SELECT * FROM ENTRIES WHERE RED LIKE '%" + query + "%'";
	grid->DeleteRows(0, grid->GetNumberRows(), true);
	sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, (void *)sqlData, &sqlErrMsg);
}

void MainDialog::OnNew(wxCommandEvent &WXUNUSED(event))
{
	wxString title = wxGetTextFromUser("Title:", "Enter new entry details", wxEmptyString);
	if (title == wxEmptyString)
	{
		wxMessageBox("Incomplete entry.", "Error", wxOK | wxICON_EXCLAMATION);
		return;
	}
	wxString user = wxGetTextFromUser("Username:", "Enter new entry details", wxEmptyString);
	if (user == wxEmptyString)
	{
		wxMessageBox("Incomplete entry.", "Error", wxOK | wxICON_EXCLAMATION);
		return;
	}
	std::string plaintext, ciphertext, recovered;
	wxMessageDialog *dial = new wxMessageDialog(NULL,
												wxT("Do you want to auto-generate password?"), wxT("Password Creation"),
												wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION);
	int res = dial->ShowModal();
	dial->Destroy();
	if (res == wxID_YES)
	{
		char temp[17];
		for (int i = 0; i < 16; i++)
		{
			temp[i] = alphanum[rand() % alphanumLength];
		}
		temp[16] = NULL;
		plaintext = temp;
	}
	else
	{
		wxString pass = wxGetTextFromUser("Password:", "Enter new entry details", wxEmptyString);
		if (pass == wxEmptyString)
		{
			wxMessageBox("Incomplete entry.", "Error", wxOK | wxICON_EXCLAMATION);
			return;
		}
		plaintext = WxToString(pass);
	}
	CryptoPP::EAX<CryptoPP::AES>::Encryption encryptor;
	encryptor.SetKeyWithIV(derived.data(), 16, derived.data() + 16, 16);
	CryptoPP::AuthenticatedEncryptionFilter ef(encryptor, new CryptoPP::StringSink(ciphertext));
	ef.Put((CryptoPP::byte *)plaintext.data(), plaintext.size());
	ef.MessageEnd();
	std::string key, iv, cipher;
	CryptoPP::HexEncoder encoder;
	encoder.Detach(new CryptoPP::StringSink(cipher));
	encoder.Put((CryptoPP::byte *)ciphertext.data(), ciphertext.size());
	encoder.MessageEnd();
	std::string stmt = "INSERT INTO ENTRIES (RED,YELLOW,GREEN) VALUES ('" + WxToString(title) + "', '" + WxToString(user) + "', '" + cipher + "');";
	sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, 0, &sqlErrMsg);
	if (sqlReturnCode != SQLITE_OK)
	{
		wxMessageBox("Database error in on new func.", "Error", wxOK | wxICON_EXCLAMATION);
		sqlite3_free(sqlErrMsg);
	}
	if (grid->GetNumberRows() != 0)
		grid->DeleteRows(0, grid->GetNumberRows(), true);
	DisplayData();
}

void MainDialog::OnDelete(wxCommandEvent &WXUNUSED(event))
{
	wxString title = wxGetTextFromUser("Title:", "Entry to delete", wxEmptyString);
	std::string query = WxToString(title);
	std::string stmt = "DELETE FROM ENTRIES WHERE RED = '" + query + "'";
	if (grid->GetNumberRows() != 0)
		grid->DeleteRows(0, grid->GetNumberRows(), true);
	DisplayData();
}

void MainDialog::OnExit(wxCommandEvent &WXUNUSED(event))
{
	Close(true);
}

void MainDialog::OnCloseWindow(wxCloseEvent &WXUNUSED(event))
{
	Show(false);
}

/*void MainDialog::ObCCall()  //Definition
{
	return ObjCCall::objectiveC_Call();  //Final Call  
}*/

/*int MainDialog::SomeMethod (void *objectiveCObject)
{
    // To invoke an Objective-C method from C++, use
    // the C trampoline function
    return MyObjectDoSomethingWith (objectiveCObject);
}*/

void MainDialog::doSomethingWithMyClass( void )
{
	_impl->init();
	_myValue = 22;
    int result = _impl->doSomethingWith( _myValue );
	wxMessageBox(std::to_string(result), "Info", wxOK | wxICON_EXCLAMATION);
    if ( result == cANSWER_TO_LIFE_THE_UNIVERSE_AND_EVERYTHING )
    {
        _impl->logMyMessage( "Hello, Arthur!" );
    }
    else
    {
        _impl->logMyMessage( "Don't worry." );
    }
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

wxBEGIN_EVENT_TABLE(TaskBarIcon, wxTaskBarIcon)
EVT_MENU(PU_RESTORE, TaskBarIcon::OnMenuRestore)
EVT_MENU(PU_NEW_ICON, TaskBarIcon::OnMenuChangeIcon)
EVT_MENU(PU_EXIT, TaskBarIcon::OnMenuExit)
wxEND_EVENT_TABLE()

void TaskBarIcon::OnMenuRestore(wxCommandEvent &)
{
	mainDialog->SetFocus();
	mainDialog->Raise();
	mainDialog->Show(true);
	if (masterKey.compare("") == 0)
	{
		wxString pwd;
		wxTextEntryDialog *dlg = new wxTextEntryDialog(mainDialog, "Password Manager", "Enter your password", "", wxOK | wxCANCEL | wxCENTRE | wxTE_PASSWORD);
		if (dlg->ShowModal() == wxID_OK)
		{
			pwd = dlg->GetValue();
		}
		else
		{
			wxMessageBox("Minimizing to taskbar", "Info", wxOK | wxICON_EXCLAMATION);
			mainDialog->Show(false);
			return;
		}
		dlg->Destroy();
		if (!pwd.empty())
		{
			unsigned int iterations = 15000;
			char purpose = 0;
			masterKey = WxToString(pwd);
			CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> kdf;
			kdf.DeriveKey(derived.data(), derived.size(), purpose, (CryptoPP::byte *)masterKey.data(), masterKey.size(), NULL, 0, iterations);
			masterKey = "NotAnEmptyString";
			DisplayData();
			VerifyKey();
			searchInput->SetFocus();
			return;
		}
		wxMessageBox("Password empty. Minimizing to taskbar.", "Error", wxOK | wxICON_EXCLAMATION);
		mainDialog->Show(false);
	}
	searchInput->SetFocus();
}

void TaskBarIcon::OnMenuChangeIcon(wxCommandEvent &) {
	icon.LoadFile(wxT("key-w.png"), wxBITMAP_TYPE_PNG);
	if (!this->SetIcon(icon, "Password Manager"))
	{
		wxLogError(wxT("Could not set icon."));
	}
    mainDialog->doSomethingWithMyClass();
	//MainDialog::ObCCall();
	//int test = MainDialog::SomeMethod(this);
	//wxMessageBox(std::to_string(test), "Alert", wxOK | wxICON_EXCLAMATION);
}

void TaskBarIcon::OnMenuExit(wxCommandEvent &)
{
	sqlite3_close(db);
	mainDialog->Destroy();
}

wxMenu *TaskBarIcon::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu;
	menu->Append(PU_RESTORE, "Show");
	menu->Append(PU_NEW_ICON, "Icon");
	menu->AppendSeparator();
	menu->Append(PU_EXIT, "Exit");
	return menu;
}