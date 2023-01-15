#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <cryptopp/cryptlib.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/eax.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include "wx/clipbrd.h"
#include "wx/grid.h"
#include "wx/taskbar.h"
#include "wxpassman.h"
#include "./sqlite3-3.35.2/sqlite3.h"
#include <stdio.h>
#include <time.h>
#include <fstream>
#include <windows.h>
#include <iostream>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <cstdlib>

bool FileExists(const std::string& name);

const char alphanum[] = "0123456789!@#$%^&*abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int alphanumLength = sizeof(alphanum) - 1;
const char* sqlData = "SQL SELECT:\n";

bool decryptPassword = false;
bool exportVals = true;
bool verifyPassword = false;
char* sqlErrMsg = 0;
CryptoPP::SecByteBlock derived(32);
MainDialog* mainDialog = NULL;
int sqlReturnCode;
sqlite3* db;
std::string masterKey = "";
std::string toDelete = "";
std::string exportPath = "";
timer_* timer;
wxGrid* grid;
wxTextCtrl* searchInput;

wxIMPLEMENT_APP(wxPassman);

bool wxPassman::OnInit()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//char* str = new char[30];

	srand(time(NULL));
	if (!wxApp::OnInit()) return false;

	if (!wxTaskBarIcon::IsAvailable()) {
		wxMessageBox("There appears to be no system tray support in your current environment. Exiting.", "Error", wxOK | wxICON_EXCLAMATION);
		return false;
	}

	WCHAR wc[MAX_PATH];
	GetModuleFileNameW(NULL, wc, MAX_PATH);
	char ch[260];
	char DefChar = ' ';
	WideCharToMultiByte(CP_ACP, 0, wc, -1, ch, 260, &DefChar, NULL);
	std::string ss(ch);
	std::string path = ss.substr(0, ss.length() - 13);
	path.append("export.true");

	if (FileExists(path))
	{
		exportVals = true;
		std::string filePathPre = path.substr(0, path.length() - 11);
		exportPath = filePathPre.append("temp.txt");
		if (!FileExists(exportPath)) {
			std::ofstream{ exportPath };
		}
		else {
			std::remove(exportPath.c_str());
			std::ofstream{ exportPath };
		}
	}

	if (FileExists("work.true"))
	{
		sqlReturnCode = sqlite3_open_v2("data.db", &db, SQLITE_OPEN_READWRITE, NULL);
	}
	else {
		sqlReturnCode = sqlite3_open_v2("D://data.db", &db, SQLITE_OPEN_READWRITE, NULL);
	}

	if (sqlReturnCode != SQLITE_OK) {
		wxMessageBox("Database cannot be found. Exiting.", "Error", wxOK | wxICON_EXCLAMATION);
		return false;
	}
	mainDialog = new MainDialog("Password Manager");
	mainDialog->SetIcon(wxICON(key));
	mainDialog->Show(false);
	return true;
}

wxBEGIN_EVENT_TABLE(MainDialog, wxDialog)
EVT_BUTTON(wxID_HIGHEST + 1, MainDialog::OnNew)
EVT_BUTTON(wxID_HIGHEST + 2, MainDialog::OnDelete)
EVT_BUTTON(wxID_HIGHEST + 3, MainDialog::OnRegen)
EVT_CLOSE(MainDialog::OnCloseWindow)
wxEND_EVENT_TABLE()


MainDialog::MainDialog(const wxString& title) : wxDialog(NULL, wxID_ANY, title) {
	timer = new timer_();
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
	grid->Bind(wxEVT_GRID_CELL_LEFT_CLICK, MainDialog::OnCellClick);
	grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, MainDialog::OnCellDClick);
	grid->Bind(wxEVT_GRID_CELL_RIGHT_CLICK, MainDialog::OnCellRightClick);
	sizerTop->Add(grid, flags.Align(wxALIGN_CENTRE));
	searchInput = new wxTextCtrl(this, -1, "", wxDefaultPosition);
	searchInput->Bind(wxEVT_TEXT, MainDialog::OnSearch);
	sizerTop->Add(searchInput, 1, wxEXPAND | wxALL, 10);
	wxSizer* const sizerBtns = new wxBoxSizer(wxHORIZONTAL);
	sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 1, wxT("&New")));
	sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 2, wxT("&Delete")));
	sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 3, wxT("&Regenerate")));
	sizerTop->Add(sizerBtns, flags.Align(wxALIGN_CENTER_HORIZONTAL));
	SetSizerAndFit(sizerTop);
	Centre();
	taskBarIcon = new TaskBarIcon();
	if (!taskBarIcon->SetIcon(wxICON(key), "Password Manager")) { wxLogError(wxT("Could not set icon.")); }
}

MainDialog::~MainDialog()
{
	delete taskBarIcon;
	delete timer;
}

bool FileExists(const std::string& name)
{
	std::ifstream f(name.c_str());
	return f.good();
}

std::string WxToString(wxString wx_string) {
	return std::string(wx_string.mb_str(wxConvUTF8));
}

wxString StringToWxString(std::string tempString) {
	wxString myWxString(tempString.c_str(), wxConvUTF8);
	return myWxString;
}

std::string Decrypt(std::string cipher) {
	std::string decoded;
	CryptoPP::HexDecoder decoder;
	decoder.Detach(new CryptoPP::StringSink(decoded));
	decoder.Put((CryptoPP::byte*)cipher.data(), cipher.size());
	decoder.MessageEnd();
	std::string secret;
	CryptoPP::EAX<CryptoPP::AES>::Decryption decrypto;
	decrypto.SetKeyWithIV(derived.data(), 16, derived.data() + 16, 16);
	CryptoPP::AuthenticatedDecryptionFilter cf(decrypto, new CryptoPP::StringSink(secret));
	cf.Put((CryptoPP::byte*)decoded.data(), decoded.size());
	cf.MessageEnd();
	return secret;
}

static int SqlExecCallback(void* data, int argc, char** argv, char** azColName) {
	int i;
	bool found = false;
	char msgBuffer[4096];

	for (i = 0; i < argc; i++) {

		if (exportVals) {
			if (strcmp(azColName[i], "GREEN") == 0) {
				std::string secret = Decrypt(std::string(argv[i]));
				wxString wSecret = wxString::FromUTF8(secret.c_str());
				std::string formatSecret = WxToString(wSecret);
				sprintf(msgBuffer, "%s = %s\n", azColName[i], secret.c_str());
			}
			else if (strcmp(azColName[i], "BLUE") == 0) {
				bool hidden = false;
				if (hidden) {
					std::string plaintext, ciphertext;
					plaintext = argv[i];
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
					sprintf(msgBuffer, "%s = %s\n", azColName[i], cipher.c_str());
				}
				else {
					std::string secret = Decrypt(std::string(argv[i]));
					wxString wSecret = wxString::FromUTF8(secret.c_str());
					std::string formatSecret = WxToString(wSecret);
					sprintf(msgBuffer, "%s = %s\n", azColName[i], secret.c_str());
				}
			}
			else {
				sprintf(msgBuffer, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
			}

			//OutputDebugStringA(msgBuffer);
			std::string logLine{ msgBuffer };
			std::ofstream log(exportPath, std::ios_base::app | std::ios_base::out);
			log << logLine;
		}

		if (decryptPassword == false) {
			if (strcmp(azColName[i], "RED") == 0) {
				grid->AppendRows(1);
				grid->SetCellValue(wxGridCellCoords(grid->GetNumberRows() - 1, 0), (wxString::FromUTF8(argv[i])));
			}
			if (strcmp(azColName[i], "YELLOW") == 0) {
				grid->SetCellValue(wxGridCellCoords(grid->GetNumberRows() - 1, 1), (wxString::FromUTF8(argv[i])));
			}
		}
		if (strcmp(azColName[i], "GREEN") == 0) {
			if (decryptPassword == true) {
				if (verifyPassword) {
					try {
						Decrypt(std::string(argv[i]));
					}
					catch (const CryptoPP::Exception& exception) {
						wxMessageBox("The key entered is incorrect. Exiting.", "Error", wxOK | wxICON_EXCLAMATION);
						mainDialog->Destroy();
						return -1;
					}
					return 0;
				}

				std::string secret = Decrypt(std::string(argv[i]));
				if (wxTheClipboard->Open()) {
					mainDialog->Show(false);
					wxTheClipboard->SetData(new wxTextDataObject(wxString::FromUTF8(secret.c_str())));
					wxTheClipboard->Close();
					timer->Start();
				}
			}
		}
	}
	return 0;
}

void DisplayData() {
	sqlReturnCode = sqlite3_exec(db, "SELECT * from ENTRIES order by RED;", SqlExecCallback, (void*)sqlData, &sqlErrMsg);
	if (sqlReturnCode != SQLITE_OK) {
		wxMessageBox("Database error in display data func.", "Error", wxOK | wxICON_EXCLAMATION);
		sqlite3_free(sqlErrMsg);
	}
}

void VerifyKey() {
	if (grid->GetNumberRows() != 0) {
		std::string clicked = WxToString(grid->GetCellValue(wxGridCellCoords(0, 0)));
		decryptPassword = true;
		verifyPassword = true;
		std::string stmt = "SELECT * FROM ENTRIES WHERE RED='" + clicked + "'";
		sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, (void*)sqlData, &sqlErrMsg);
		decryptPassword = false;
		verifyPassword = false;
	}
}

void MainDialog::OnCellClick(wxGridEvent& event) {
	std::string clicked = WxToString(grid->GetCellValue(wxGridCellCoords(event.GetRow(), 0)));
	grid->SelectRow(event.GetRow());
	toDelete = clicked;
}

void MainDialog::OnCellDClick(wxGridEvent& event) {
	std::string clicked = WxToString(grid->GetCellValue(wxGridCellCoords(event.GetRow(), 0)));
	grid->SelectRow(event.GetRow());
	decryptPassword = true;
	std::string stmt = "SELECT * FROM ENTRIES WHERE RED='" + clicked + "'";
	sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, (void*)sqlData, &sqlErrMsg);
	decryptPassword = false;
}

void MainDialog::OnCellRightClick(wxGridEvent& event)
{
	std::string clicked = WxToString(grid->GetCellValue(wxGridCellCoords(event.GetRow(), 1)));
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxTextDataObject(wxString::FromUTF8(clicked.c_str())));
		wxTheClipboard->Close();
	}
}

void MainDialog::OnSearch(wxCommandEvent& event) {
	std::string query = std::string(searchInput->GetValue());
	if (query == "") {
		grid->DeleteRows(0, grid->GetNumberRows(), true);
		DisplayData();
		return;
	}
	std::string stmt = "SELECT * FROM ENTRIES WHERE RED LIKE '%" + query + "%'";
	grid->DeleteRows(0, grid->GetNumberRows(), true);
	sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, (void*)sqlData, &sqlErrMsg);
}

void MainDialog::OnNew(wxCommandEvent& WXUNUSED(event)) {
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
	std::string plaintext, ciphertext;
	wxMessageDialog* dial = new wxMessageDialog(NULL,
		wxT("Do you want to auto-generate password?"), wxT("Password Creation"),
		wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION);
	int res = dial->ShowModal();
	dial->Destroy();
	if (res == wxID_YES) {
		char temp[17];
		for (int i = 0; i < 16; i++) {
			temp[i] = alphanum[rand() % alphanumLength];
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
		plaintext = WxToString(pass);
	}
	CryptoPP::EAX<CryptoPP::AES>::Encryption encryptor;
	encryptor.SetKeyWithIV(derived.data(), 16, derived.data() + 16, 16);
	CryptoPP::AuthenticatedEncryptionFilter ef(encryptor, new CryptoPP::StringSink(ciphertext));
	ef.Put((CryptoPP::byte*)plaintext.data(), plaintext.size());
	ef.MessageEnd();
	std::string cipher;
	CryptoPP::HexEncoder encoder;
	encoder.Detach(new CryptoPP::StringSink(cipher));
	encoder.Put((CryptoPP::byte*)ciphertext.data(), ciphertext.size());
	encoder.MessageEnd();

	std::string plaintext2, ciphertext2;
	wxString url = wxGetTextFromUser("URL:", "Enter new entry details", wxEmptyString);
	if (url == wxEmptyString) {
		wxMessageBox("Incomplete entry.", "Error", wxOK | wxICON_EXCLAMATION);
		return;
	}
	plaintext2 = WxToString(url);
	CryptoPP::EAX<CryptoPP::AES>::Encryption encryptor2;
	encryptor2.SetKeyWithIV(derived.data(), 16, derived.data() + 16, 16);
	CryptoPP::AuthenticatedEncryptionFilter ef2(encryptor2, new CryptoPP::StringSink(ciphertext2));
	ef2.Put((CryptoPP::byte*)plaintext2.data(), plaintext2.size());
	ef2.MessageEnd();
	std::string cipher2;
	CryptoPP::HexEncoder encoder2;
	encoder2.Detach(new CryptoPP::StringSink(cipher2));
	encoder2.Put((CryptoPP::byte*)ciphertext2.data(), ciphertext2.size());
	encoder2.MessageEnd();

	std::string stmt = "INSERT INTO ENTRIES (RED,BLUE, YELLOW,GREEN) VALUES ('" + WxToString(title) + "', '" + cipher2 + "', '" + WxToString(user) + "', '" + cipher + "');";
	sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, 0, &sqlErrMsg);
	if (sqlReturnCode != SQLITE_OK) {
		wxMessageBox("Database error in on new func.", "Error", wxOK | wxICON_EXCLAMATION);
		sqlite3_free(sqlErrMsg);
	}
	if (grid->GetNumberRows() != 0) grid->DeleteRows(0, grid->GetNumberRows(), true);
	DisplayData();
}

void MainDialog::OnDelete(wxCommandEvent& WXUNUSED(event)) {
	wxString title = "";
	if (toDelete != "") {
		title = StringToWxString(toDelete);
		wxString caption = "Are you sure you want to delete " + title + "?";
		wxMessageDialog* dial = new wxMessageDialog(NULL, caption, wxT("Password Creation"),
			wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION);
		int res = dial->ShowModal();
		dial->Destroy();
		if (res == wxID_NO) {
			return;
		}
	}
	else {
		title = wxGetTextFromUser("Title:", "Entry to delete", wxEmptyString);
	}
	std::string query = WxToString(title);
	std::string stmt = "DELETE FROM ENTRIES WHERE RED = '" + query + "'";
	sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, 0, &sqlErrMsg);
	if (grid->GetNumberRows() != 0) grid->DeleteRows(0, grid->GetNumberRows(), true);
	toDelete = "";
	DisplayData();
}

void MainDialog::OnRegen(wxCommandEvent& WXUNUSED(event)) {
	wxString title = "";
	if (toDelete != "") {
		title = StringToWxString(toDelete);
		wxString caption = "Are you sure you want to regenerate password for " + title + "?";
		wxMessageDialog* dial = new wxMessageDialog(NULL, caption, wxT("Password Creation"),
			wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION);
		int res = dial->ShowModal();
		dial->Destroy();
		if (res == wxID_NO) {
			return;
		}
		else {

			std::string query = WxToString(title);
			std::string plaintext, ciphertext;
			wxMessageDialog* dial = new wxMessageDialog(NULL,
				wxT("Do you want to auto-generate password?"), wxT("Password Creation"),
				wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION);
			int res = dial->ShowModal();
			dial->Destroy();
			if (res == wxID_YES) {
				char temp[17];
				for (int i = 0; i < 16; i++) {
					temp[i] = alphanum[rand() % alphanumLength];
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
				plaintext = WxToString(pass);
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
			std::string stmt = "UPDATE ENTRIES SET GREEN = '" + cipher + "' WHERE RED = '" + query + "'";
			sqlReturnCode = sqlite3_exec(db, stmt.c_str(), SqlExecCallback, 0, &sqlErrMsg);
			if (sqlReturnCode != SQLITE_OK) {
				wxMessageBox("Database error in on new func.", "Error", wxOK | wxICON_EXCLAMATION);
				sqlite3_free(sqlErrMsg);
			}
		}
	}
}

void MainDialog::OnExit(wxCommandEvent& WXUNUSED(event)) {
	Close(true);
}

void MainDialog::OnCloseWindow(wxCloseEvent& WXUNUSED(event)) {
	Show(false);
}

enum { PU_EXIT };

wxBEGIN_EVENT_TABLE(TaskBarIcon, wxTaskBarIcon)
EVT_MENU(PU_EXIT, TaskBarIcon::OnMenuExit)
EVT_TASKBAR_LEFT_DOWN(TaskBarIcon::OnLeftButtonClick)
wxEND_EVENT_TABLE()

void TaskBarIcon::OnMenuExit(wxCommandEvent&)
{
	sqlite3_close(db);
	mainDialog->Destroy();
}

wxMenu* TaskBarIcon::CreatePopupMenu()
{
	wxMenu* menu = new wxMenu;
	menu->Append(PU_EXIT, wxT("E&xit"));
	return menu;
}

void TaskBarIcon::OnLeftButtonClick(wxTaskBarIconEvent&)
{
	mainDialog->Show(true);
	mainDialog->Raise();
	if (masterKey.compare("") == 0) {
		wxString pwd;
		wxTextEntryDialog* dlg = new wxTextEntryDialog(mainDialog, "Password Manager", "Enter your password", "", wxOK | wxCANCEL | wxCENTRE | wxTE_PASSWORD);
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
		if (!pwd.empty()) {
			unsigned int iterations = 15000;
			char purpose = 0;
			masterKey = WxToString(pwd);
			CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> kdf;
			kdf.DeriveKey(derived.data(), derived.size(), purpose, (CryptoPP::byte*)masterKey.data(), masterKey.size(), NULL, 0, iterations);
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