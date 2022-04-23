#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/clipbrd.h>
#include <wx/font.h>
#include <wx/grid.h>
#include <wx/string.h>
#include <wx/taskbar.h>
#include <../cryptopp/cryptlib.h>
#include <../cryptopp/aes.h>
#include <../cryptopp/modes.h>
#include <../cryptopp/filters.h>
#include <../cryptopp/pwdbased.h>
#include <../cryptopp/eax.h>
#include <../cryptopp/hex.h>
#include <../cryptopp/sha.h>
#include "sqlite3-3.35.2/sqlite3.h"

#include <sunset.h>
#include <chrono>
#include <thread>
#include <time.h>
#include <fstream>

#include "wxpassman.h"

#define LATITUDE 45.4127
#define LONGITUDE -75.6887
#define TIMEZONE -4 // DST_OFFSET

const char alphanum[] = "0123456789!@#$%^&*abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int alphanumLength = sizeof(alphanum) - 1;
const char *sqlData = "SQL SELECT:\n";

bool darkmode = false;
bool debugLog = false;
bool dateLogged = false;
bool decryptPassword = false;
bool verifyPassword = false;
char *sqlErrMsg = 0;
CryptoPP::SecByteBlock derived(32);
MainDialog *mainDialog = NULL;
int sqlReturnCode;
sqlite3 *db;
std::string masterKey = "";
std::string toDelete = "";
ClipboardTimer *clipboardTimer;
IconTimer *iconTimer;
SunSet sun;
time_t sunriseTime;
time_t sunsetTime;
wxGrid *grid;
wxIcon icon;
wxTextCtrl *searchInput;

static void Log(std::string line);
static void LogDate(const char a[], const char b[]);
static int SqlExecCallback(void *data, int argc, char **argv, char **azColName);
bool FileExists(const std::string &name);
std::string Decrypt(std::string cipher);
std::string WxToString(wxString wx_string);
void DisplayData();
void IconTimerInit();
void VerifyKey();
wxString StringToWxString(std::string tempString);

wxIMPLEMENT_APP(wxPassman);

bool wxPassman::OnInit()
{
	srand(time(NULL));

	if (!wxApp::OnInit())
	{
		return false;
	}

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


	if (FileExists("debug.true"))
	{
		debugLog = true;
	}
	Log("\n\n\n--- Application start ---");

	wxImage::AddHandler(new wxPNGHandler);
	icon.LoadFile(wxT("key-b.png"), wxBITMAP_TYPE_PNG);
	mainDialog = new MainDialog("Password Manager");
	mainDialog->SetIcon(icon);
	mainDialog->Show(false);

	iconTimer = new IconTimer();
	iconTimer->startOnce(0001);
	return true;
}

wxBEGIN_EVENT_TABLE(MainDialog, wxDialog)
	EVT_BUTTON(wxID_HIGHEST + 1, MainDialog::OnNew)
	EVT_BUTTON(wxID_HIGHEST + 2, MainDialog::OnDelete)
	EVT_BUTTON(wxID_HIGHEST + 3, MainDialog::OnRegen)
	EVT_BUTTON(wxID_EXIT, MainDialog::OnExit)
	EVT_CLOSE(MainDialog::OnCloseWindow)
wxEND_EVENT_TABLE()

MainDialog::MainDialog(const wxString &title) : wxDialog(NULL, wxID_ANY, title, wxPoint(10, 735))
{
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
	grid->Bind(wxEVT_GRID_CELL_LEFT_CLICK, MainDialog::OnCellClick);
	grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, MainDialog::OnCellDClick);
	grid->Bind(wxEVT_GRID_CELL_RIGHT_CLICK, MainDialog::OnCellRightClick);
	sizerTop->Add(grid, flags.Align(wxALIGN_CENTRE));

	searchInput = new wxTextCtrl(this, -1, "", wxDefaultPosition);
	searchInput->Bind(wxEVT_TEXT, MainDialog::OnSearch);
	sizerTop->Add(searchInput, 1, wxEXPAND | wxALL, 10);

	wxSizer *const sizerBtns = new wxBoxSizer(wxHORIZONTAL);
	sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 1, wxT("&New")));
	sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 2, wxT("&Delete")));
	sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 3, wxT("&Regenerate")));
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
	Log("Clean up started");
	sqlite3_close(db);
	delete taskBarIcon;
	delete clipboardTimer;
	delete iconTimer;
	Log("Clean up completed");
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
	wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("Do you want to auto-generate password?"), wxT("Password Creation"), wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION);
	int res = dial->ShowModal();
	dial->Destroy();

	if (res == wxID_YES)
	{
		char temp[17];
		for (int i = 0; i < 16; i++)
		{
			temp[i] = alphanum[rand() % alphanumLength];
		}
		temp[16] = '\0';
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
	{
		grid->DeleteRows(0, grid->GetNumberRows(), true);
	}
	DisplayData();
}

void MainDialog::OnDelete(wxCommandEvent &WXUNUSED(event))
{
	wxString title = "";
	if (toDelete != "")
	{
		title = StringToWxString(toDelete);
		wxString caption = "Are you sure you want to delete " + title + "?";
		wxMessageDialog *dial = new wxMessageDialog(NULL, caption, wxT("Password Creation"),
													wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION);
		int res = dial->ShowModal();
		dial->Destroy();
		if (res == wxID_NO)
		{
			return;
		}
	}
	else
	{
		title = wxGetTextFromUser("Title:", "Entry to delete", wxEmptyString);
	}
	
	std::string query = WxToString(title);
	std::string stmt = "DELETE FROM ENTRIES WHERE RED = '" + query + "'";
	sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, 0, &sqlErrMsg);
	if (grid->GetNumberRows() != 0)
	{
		grid->DeleteRows(0, grid->GetNumberRows(), true);
	}
	toDelete = "";
	DisplayData();
}

void MainDialog::OnRegen(wxCommandEvent &WXUNUSED(event))
{
	wxString title = "";
	if (toDelete != "")
	{
		title = StringToWxString(toDelete);
		wxString caption = "Are you sure you want to regenerate password for " + title + "?";
		wxMessageDialog *dial = new wxMessageDialog(NULL, caption, wxT("Password Creation"),
													wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION);
		int res = dial->ShowModal();
		dial->Destroy();
		if (res == wxID_NO)
		{
			return;
		}
		else
		{
			std::string query = WxToString(title);
			std::string plaintext, ciphertext, recovered;
			char temp[17];
			for (int i = 0; i < 16; i++)
			{
				temp[i] = alphanum[rand() % alphanumLength];
			}
			temp[16] = '\0';
			plaintext = temp;

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

			std::string stmt = "UPDATE ENTRIES SET GREEN = '" + cipher + "' WHERE RED = '" + query + "'";
			sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, 0, &sqlErrMsg);
			if (sqlReturnCode != SQLITE_OK)
			{
				wxMessageBox("Database error in on new func.", "Error", wxOK | wxICON_EXCLAMATION);
				sqlite3_free(sqlErrMsg);
			}
		}
	}
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

void MainDialog::IconTimerNotify()
{
	time_t currTime = time(0);
	struct tm tm_currTime;
	localtime_r(&currTime, &tm_currTime);
	mktime(&tm_currTime);

	struct tm tm_midnightTime;
	localtime_r(&currTime, &tm_midnightTime);
	tm_midnightTime.tm_sec = tm_midnightTime.tm_min = tm_midnightTime.tm_hour = 0;
	mktime(&tm_midnightTime);

	sun.setPosition(LATITUDE, LONGITUDE, TIMEZONE);
	sun.setCurrentDate(tm_currTime.tm_year + 1900, tm_currTime.tm_mon + 1, tm_currTime.tm_mday);
	sun.setTZOffset(TIMEZONE);

	double sunrise = sun.calcSunrise();
	double sunset = sun.calcSunset();

	time_t midnightTime = mktime(&tm_midnightTime);
	sunriseTime = midnightTime + (sunrise * 60);
	sunsetTime = midnightTime + (sunset * 60);

	double sunriseDiff = difftime(sunriseTime, currTime);
	double sunsetDiff = difftime(sunsetTime, currTime);

	Log("--- Initialize icon timer");
	Log("Current time: " + std::to_string(tm_currTime.tm_hour) + ":" + std::to_string(tm_currTime.tm_min) + ":" + std::to_string(tm_currTime.tm_sec) + " " + std::to_string(tm_currTime.tm_mday) + "/" + std::to_string(tm_currTime.tm_mon + 1) + "/" + std::to_string(tm_currTime.tm_year + 1900));
	Log("sunriseDiff: " + std::to_string((sunriseDiff / 60) / 60) + " sunsetDiff: " + std::to_string((sunsetDiff / 60) / 60));

    if (!dateLogged) {
	    char buff[20];
	    struct tm *timeinfo;
	    timeinfo = localtime(&sunriseTime);
	    strftime(buff, sizeof(buff), "%b %d %H:%M", timeinfo);
	    LogDate("sunriseTime: ", buff);
	    timeinfo = localtime(&sunsetTime);
	    strftime(buff, sizeof(buff), "%b %d %H:%M", timeinfo);
	    LogDate("sunsetTime: ", buff);
		dateLogged = true;
	}

	if (sunriseDiff < 0 && sunsetDiff < 0)
	{ // after sunset before midnight, add a day to sunrise for next day
		Log("after sunset before midnight");
		sunriseTime = midnightTime + (sunrise * 60) + (24 * 60 * 60);
		sunriseDiff = difftime(sunriseTime, currTime);
		Log("next timer tick in " + std::to_string((sunriseDiff / 60) / 60));
		iconTimer->startOnce(sunriseDiff * 1000);
		darkmode = true;
	}
	else if (sunriseDiff < 0)
	{ // between sunrise and sunset, set timer to sunset
		Log("between sunrise and sunset");
		Log("next timer tick in " + std::to_string((sunsetDiff / 60) / 60));
		iconTimer->startOnce(std::abs(sunriseDiff * 1000)); // in millis
		darkmode = false;
	}
	else
	{ // before sunrise
		Log("before sunrise");
		Log("next timer tick in " + std::to_string((sunriseDiff / 60) / 60));
		iconTimer->startOnce(sunriseDiff * 1000);
		darkmode = true;
	}

	if (darkmode)
	{
		Log("darkmode=true");
		icon.LoadFile(wxT("key-w.png"), wxBITMAP_TYPE_PNG);
	}
	else
	{
		Log("darkmode=false");
		icon.LoadFile(wxT("key-b.png"), wxBITMAP_TYPE_PNG);
	}
	mainDialog->taskBarIcon->SetIcon(icon, "Password Manager");
}

void MainDialog::OnCellClick(wxGridEvent &event)
{
	std::string clicked = WxToString(grid->GetCellValue(wxGridCellCoords(event.GetRow(), 0)));
	grid->SelectRow(event.GetRow());
	toDelete = clicked;
}

void MainDialog::OnCellDClick(wxGridEvent &event)
{
	std::string clicked = WxToString(grid->GetCellValue(wxGridCellCoords(event.GetRow(), 0)));
	grid->SelectRow(event.GetRow());
	decryptPassword = true;

	std::string stmt = "SELECT * FROM ENTRIES WHERE RED='" + clicked + "'";
	sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, (void *)sqlData, &sqlErrMsg);
	decryptPassword = false;
}

void MainDialog::OnCellRightClick(wxGridEvent &event)
{
	std::string clicked = WxToString(grid->GetCellValue(wxGridCellCoords(event.GetRow(), 1)));
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(wxString::FromUTF8(clicked.c_str())));
		wxTheClipboard->Close();
	}
}

void MainDialog::OnCloseWindow(wxCloseEvent &WXUNUSED(event))
{
	Show(false);
}

void MainDialog::OnExit(wxCommandEvent &WXUNUSED(event))
{
	Log("OnExit");
	Close(true);
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

bool FileExists(const std::string &name)
{
	std::ifstream f(name.c_str());
	return f.good();
}

// https://stackoverflow.com/questions/2393345/how-to-append-text-to-a-text-file-in-c
static void Log(std::string line)
{
	if (debugLog)
	{
		std::string filepath = "/Users/luv/Desktop/wxpassman.log";
		std::ofstream file;

		file.open(filepath, std::ios::out | std::ios::app);
		if (file.fail())
			throw std::ios_base::failure(std::strerror(errno));

		// make sure write fails with exception if something is wrong
		file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

		std::cout << line << std::endl;
		file << line << std::endl;
	}
}

static void LogDate(const char a[], const char b[])
{
	if (debugLog)
	{
		std::string filepath = "/Users/luv/Desktop/wxpassman.log";
		std::ofstream file;

		file.open(filepath, std::ios::out | std::ios::app);
		if (file.fail())
			throw std::ios_base::failure(std::strerror(errno));

		file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

		std::cout << a << " " << b << std::endl;
		file << a << " " << b << std::endl;
	}
}

static int SqlExecCallback(void *data, int argc, char **argv, char **azColName)
{
	int i;
	// char msgBuffer[4096];
	for (i = 0; i < argc; i++)
	{
		/*if (strcmp(azColName[i], "GREEN") == 0) {
			std::string secret = Decrypt(std::string(argv[i]));
			wxString wSecret = wxString::FromUTF8(secret.c_str());
			std::string formatSecret = WxToString(wSecret);
			sprintf(msgBuffer, "%s = %s\n", azColName[i], secret.c_str());
		}
		else {
			sprintf(msgBuffer, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		}
		std::cout << msgBuffer << std::endl;*/
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

std::string WxToString(wxString wx_string)
{
	return std::string(wx_string.mb_str(wxConvUTF8));
}

wxString StringToWxString(std::string tempString)
{
	wxString myWxString(tempString.c_str(), wxConvUTF8);
	return myWxString;
}

enum
{
	PU_RESTORE = 10001,
	PU_EXIT,
};

wxBEGIN_EVENT_TABLE(TaskBarIcon, wxTaskBarIcon)
	EVT_MENU(PU_RESTORE, TaskBarIcon::OnMenuRestore)
	EVT_MENU(PU_EXIT, TaskBarIcon::OnMenuExit)
wxEND_EVENT_TABLE()

wxMenu *TaskBarIcon::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu;
	menu->Append(PU_RESTORE, "Show");
	menu->AppendSeparator();
	menu->Append(PU_EXIT, "Exit");
	return menu;
}

void TaskBarIcon::OnMenuExit(wxCommandEvent &)
{
	mainDialog->Destroy();
}

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
