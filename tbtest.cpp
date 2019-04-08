#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "./sample.xpm" // the application icon (under Windows and OS/2 it is in resources)
#endif

#include <wx/timer.h>
#include "wx/taskbar.h"
#include <wx/textdlg.h>
#include <wx/grid.h>
#include <wx/clipbrd.h>
#include "tbtest.h"
#include <wx/textctrl.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/eax.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

static MyDialog *gs_dialog = NULL;
std::string master_key = "";
wxGrid *grid;
wxTextCtrl *search_input;
sqlite3 *db;
char *zErrMsg = 0;
int rc;
char *sql;
const char* data = "SQL SELECT:\n";
CryptoPP::SecByteBlock derived(32);
bool getPassword = false;

std::string wx_to_string(wxString wx_string) {
    return std::string(wx_string.mb_str(wxConvUTF8));
}

IMPLEMENT_APP(MyApp)

class timer_ :public wxTimer
{
public:
   
    timer_():wxTimer()
    {
    }
    void Notify()
    {
        wxTheClipboard->Clear();
    }
    void start()
    {
        wxTimer::StartOnce(3000);
    }
};

bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    if ( !wxTaskBarIcon::IsAvailable() )
    {
        wxMessageBox
        (
            "There appears to be no system tray support in your current environment. You are stupid for trying.",
            "Warning",
            wxOK | wxICON_EXCLAMATION
        );
    }

    // Create the main window
    rc = sqlite3_open("data.db", &db);
    gs_dialog = new MyDialog(wxT("Password Manager"));
    //gs_dialog->Show(true); // --------------------------------------------- HIDE WINDOW ON OPEN
    return true;
}

wxBEGIN_EVENT_TABLE(MyDialog, wxDialog)
    EVT_BUTTON(wxID_HIGHEST + 1, MyDialog::OnNew)
    EVT_BUTTON(wxID_HIGHEST + 2, MyDialog::OnDelete)
    EVT_CLOSE(MyDialog::OnCloseWindow)
wxEND_EVENT_TABLE()

MyDialog::MyDialog(const wxString& title): wxDialog(NULL, wxID_ANY, title)
{
    wxSizer * const sizerTop = new wxBoxSizer(wxVERTICAL);
    wxSizerFlags flags;
    flags.Border(wxALL, 10);
        
    grid = new wxGrid(this, -1, wxEXPAND|wxALL);
    grid->CreateGrid(0,2);
    grid->SetColLabelValue(0, _("Title")); 
    grid->SetColLabelValue(1, _("Username")); 
    grid->SetMinSize(wxSize(400, 280));
    grid->SetColSize(0, 150);
    grid->SetColSize(1, 150);
    grid->SetSelectionMode(wxGrid::wxGridSelectRows);
    grid->SetEditable(false);
    grid->AutoSizeRows();
    grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, MyDialog::OnCellClick);
    sizerTop->Add(grid, flags.Align(wxALIGN_CENTRE));
    
    search_input = new wxTextCtrl(this, -1, "", wxDefaultPosition);
    search_input->Bind(wxEVT_TEXT, MyDialog::OnSearch);
    sizerTop->Add(search_input, 1, wxEXPAND | wxALL, 10); 

    wxSizer * const sizerBtns = new wxBoxSizer(wxHORIZONTAL);
    sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 1, wxT("&New")));
    sizerBtns->Add(new wxButton(this, wxID_HIGHEST + 2, wxT("&Delete")));

    sizerTop->Add(sizerBtns, flags.Align(wxALIGN_CENTER_HORIZONTAL));

    SetSizerAndFit(sizerTop);
    Centre();

    m_taskBarIcon = new MyTaskBarIcon();

    if ( !m_taskBarIcon->SetIcon(wxICON(sample),
                                 "Password Manager") )
    {
        wxLogError(wxT("Could not set icon."));
    }

#if defined(__WXOSX__) && wxOSX_USE_COCOA
    m_dockIcon = new MyTaskBarIcon(wxTBI_DOCK);
    if ( !m_dockIcon->SetIcon(wxICON(sample)) )
    {
        wxLogError(wxT("Could not set icon."));
    }
#endif
}

MyDialog::~MyDialog()

{
    delete m_taskBarIcon;
}

std::string my_decrypt(std::string cipher) {

    std::string decoded;
    CryptoPP::HexDecoder decoder;
    decoder.Detach(new CryptoPP::StringSink(decoded));
    decoder.Put( (CryptoPP::byte*)cipher.data(), cipher.size() );
    decoder.MessageEnd();
    std::string test;
    CryptoPP::EAX<CryptoPP::AES>::Decryption decrypto;
    decrypto.SetKeyWithIV(derived.data(), 16, derived.data() + 16, 16);
    CryptoPP::AuthenticatedDecryptionFilter cf(decrypto, new CryptoPP::StringSink(test));
    cf.Put((CryptoPP::byte*)decoded.data(), decoded.size());
    cf.MessageEnd();
    return test;
}

static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   bool found = false; 
   for(i = 0; i<argc; i++){
     // printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      if(getPassword == false) {
      if(strcmp(azColName[i], "RED") == 0) {
        grid->AppendRows(1);
        grid->SetCellValue(wxGridCellCoords(grid->GetNumberRows()-1, 0), (wxString::FromUTF8(argv[i])));
      }
      if(strcmp(azColName[i], "YELLOW") == 0) {
        grid->SetCellValue(wxGridCellCoords(grid->GetNumberRows()-1, 1), (wxString::FromUTF8(argv[i])));
      }
      }
      if(strcmp(azColName[i], "GREEN") == 0) {
        if(getPassword == true) {
            try { 
                    my_decrypt(std::string(argv[i])); 
                } 
            catch(const CryptoPP::Exception& exception) { 
                    std::cout << "Caught exception: " << exception.what() << '\n';
                    wxMessageBox("The key entered is incorrect - restart application", "Error", wxOK | wxICON_EXCLAMATION);
                    gs_dialog->Destroy();
                    return -1;
                }
            std::string test = my_decrypt(std::string(argv[i])); 
            if (wxTheClipboard->Open()) { 
               gs_dialog->Show(false);
               wxTheClipboard->SetData(new wxTextDataObject(wxString::FromUTF8(test.c_str())));
               wxTheClipboard->Close();
               timer_* timer = new timer_();
               timer->start();
            }
        }
      }
   }
   return 0;
}

void displayData() {
    /* Create SQL statement */
    /* Execute SQL statement */
    rc = sqlite3_exec(db, "SELECT * from ENTRIES order by RED;", callback, (void*)data, &zErrMsg);
       if( rc != SQLITE_OK ) {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    } else {
      fprintf(stdout, "Operation done successfully\n");
    } 
}

void MyDialog::OnCellClick(wxGridEvent& event) {
   
    std::string clicked = wx_to_string(grid->GetCellValue(wxGridCellCoords(event.GetRow(), 0)));
    getPassword = true;
    std::string stmt = "SELECT * FROM ENTRIES WHERE RED='"+clicked+"'";
    rc = sqlite3_exec(db, stmt.c_str(), callback, (void*)data, &zErrMsg);
    getPassword = false;
}

void MyDialog::OnSearch(wxCommandEvent& event) {

    std::string query = std::string(search_input->GetValue());

    if(query == "") { 
        grid->DeleteRows(0, grid->GetNumberRows(), true);
        displayData();
        return;
    }

    std::string stmt = "SELECT * FROM ENTRIES WHERE RED LIKE '%"+query+"%'";
    grid->DeleteRows(0, grid->GetNumberRows(), true);
    rc = sqlite3_exec(db, stmt.c_str(), callback, (void*)data, &zErrMsg);
}

void MyDialog::OnNew(wxCommandEvent& WXUNUSED(event))
{
 
    wxString title = wxGetTextFromUser("Title:", "Enter new entry details", wxEmptyString);
    wxString user = wxGetTextFromUser("Username:", "Enter new entry details", wxEmptyString);
    wxString pass = wxGetTextFromUser("Password:", "Enter new entry details", wxEmptyString);
    
    std::string plaintext = wx_to_string(pass), ciphertext, recovered;
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

    std::string stmt = "INSERT INTO ENTRIES (RED,YELLOW,GREEN) VALUES ('"+wx_to_string(title)+"', '"+wx_to_string(user)+"', '"+cipher+"');";

    rc = sqlite3_exec(db, stmt.c_str(), callback, 0, &zErrMsg);
    if( rc != SQLITE_OK ) {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    } else {
      fprintf(stdout, "Operation done successfully\n");
    } 
    if(grid->GetNumberRows() != 0) grid->DeleteRows(0, grid->GetNumberRows(), true);
    displayData();
}

void MyDialog::OnDelete(wxCommandEvent& WXUNUSED(event))
{
    wxString title = wxGetTextFromUser("Title:", "Entry to delete", wxEmptyString);
    std::string query = wx_to_string(title);
    std::string stmt = "DELETE FROM ENTRIES WHERE RED = '"+query+"'";
    rc = sqlite3_exec(db, stmt.c_str(), callback, 0, &zErrMsg);
    if(grid->GetNumberRows() != 0) grid->DeleteRows(0, grid->GetNumberRows(), true);
    displayData(); 
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
    PU_EXIT,
};

wxBEGIN_EVENT_TABLE(MyTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(PU_EXIT,    MyTaskBarIcon::OnMenuExit)
    EVT_TASKBAR_LEFT_DCLICK  (MyTaskBarIcon::OnLeftButtonDClick)
wxEND_EVENT_TABLE()

void MyTaskBarIcon::OnMenuExit(wxCommandEvent& )
{
    sqlite3_close(db);
    gs_dialog->Destroy();
}

// Overridables
wxMenu *MyTaskBarIcon::CreatePopupMenu()
{
    wxMenu *menu = new wxMenu;
    /* OSX has built-in quit menu for the dock menu, but not for the status item */
#ifdef __WXOSX__ 
    if ( OSXIsStatusItem() )
#endif
    {
        menu->Append(PU_EXIT, wxT("E&xit"));
    }
    return menu;
}

void MyTaskBarIcon::OnLeftButtonDClick(wxTaskBarIconEvent&)
{

    gs_dialog->Show(true);
    if(master_key.compare("") == 0) {
        wxString pwd = wxGetPasswordFromUser("Enter key:", "Password Manager", wxEmptyString);
        if (!pwd.empty())
        {
            unsigned int iterations = 15000;
            char purpose = 0; 
            master_key = wx_to_string(pwd);
            CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> kdf;
            kdf.DeriveKey(derived.data(), derived.size(), purpose, (CryptoPP::byte*)master_key.data(), master_key.size(), NULL, 0, iterations);
            displayData();
            search_input->SetFocus();    
            return;
        } 
        wxMessageBox("Exiting - possible causes:\n> Blank key entered\n> Cancel button pushed", "Error", wxOK | wxICON_EXCLAMATION);
        gs_dialog->Destroy();
    }
    search_input->SetFocus();
}

