#include "encIDEApp.h"

#include "encIDEMain.h"
#include <wx/msgdlg.h>
#include <sstream>

//(*InternalHeaders(encIDEFrame)
#include <wx/intl.h>
//*)

std::string compPath = "";
std::string riscvTarget = "";
std::string riscvPath = "";
std::string fileName = "";

std::string replaceStr(std::string sAll, std::string sStringToRemove, std::string sStringToInsert)
{
   int iLength = sStringToRemove.length();
   size_t index = 0;
   while (true)
   {
      /* Locate the substring to replace. */
      index = sAll.find(sStringToRemove, index);
      if (index == std::string::npos)
         break;

      /* Make the replacement. */
      sAll.replace(index, iLength, sStringToInsert);

      /* Advance index forward so the next iteration doesn't pick it up as well. */
      index += iLength;
   }
   return sAll;
}

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(encIDEFrame)
const long encIDEFrame::ID_MENUITEM1 = wxNewId();
const long encIDEFrame::idMenuAbout = wxNewId();
const long encIDEFrame::ID_STATUSBAR1 = wxNewId();
//*)

const long encIDEFrame::idSaveFile = wxNewId();
const long encIDEFrame::idOpenFile = wxNewId();
const long encIDEFrame::idComp = wxNewId();
const long encIDEFrame::idCompPath = wxNewId();
const long encIDEFrame::idRiscvTarget = wxNewId();
const long encIDEFrame::idRiscvPath = wxNewId();
const long encIDEFrame::idEncrypt = wxNewId();
const long encIDEFrame::idCheckCompStr = wxNewId();

const long encIDEFrame::ID_CHOICE1 = wxNewId();
const long encIDEFrame::ID_CHOICE2 = wxNewId();

const long encIDEFrame::ID_STYLEDTEXTCTRL1 = wxNewId();

enum
{
    MARGIN_LINE_NUMBERS,
    MARGIN_FOLD
};

BEGIN_EVENT_TABLE(encIDEFrame,wxFrame)
    //(*EventTable(encIDEFrame)
    //*)
END_EVENT_TABLE()

encIDEFrame::encIDEFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(encIDEFrame)
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItem1;
    wxMenu* Menu1;
    wxMenuBar* MenuBar1;
    wxMenu* Menu2;

    Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem1 = new wxMenuItem(Menu1, ID_MENUITEM1, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu2 = new wxMenu();
    MenuItem2 = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItem2);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -1 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    StatusBar1->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);

    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&encIDEFrame::OnQuit);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&encIDEFrame::OnAbout);
    //*)

    SetClientSize(wxSize(713,450));

    this->CenterOnScreen();

    Choice1 = new wxChoice(this, ID_CHOICE1, wxPoint(320,16), wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice2 = new wxChoice(this, ID_CHOICE2, wxPoint(440,16), wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));

    StyledTextCtrl1 = new wxStyledTextCtrl(this, ID_STYLEDTEXTCTRL1, wxPoint(8,56), wxSize(696,384), 0, _T("ID_STYLEDTEXTCTRL1"));

    Choice1->Append("rv32");
    Choice1->Append("rv64");

    Choice2->Append("gc");
    Choice2->Append("i");

    StyledTextCtrl1->StyleClearAll();
    StyledTextCtrl1->SetLexer(wxSTC_LEX_CPP);

    StyledTextCtrl1->SetMarginWidth (MARGIN_LINE_NUMBERS, 50);
    StyledTextCtrl1->StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour (75, 75, 75) );
    StyledTextCtrl1->StyleSetBackground (wxSTC_STYLE_LINENUMBER, wxColour (220, 220, 220));
    StyledTextCtrl1->SetMarginType (MARGIN_LINE_NUMBERS, wxSTC_MARGIN_NUMBER);

    StyledTextCtrl1->SetMarginType (MARGIN_FOLD, wxSTC_MARGIN_SYMBOL);
    StyledTextCtrl1->SetMarginWidth(MARGIN_FOLD, 15);
    StyledTextCtrl1->SetMarginMask (MARGIN_FOLD, wxSTC_MASK_FOLDERS);
    StyledTextCtrl1->StyleSetBackground(MARGIN_FOLD, wxColor(200, 200, 200) );
    StyledTextCtrl1->SetMarginSensitive(MARGIN_FOLD, true);

    StyledTextCtrl1->SetProperty (wxT("fold"),         wxT("1") );
    StyledTextCtrl1->SetProperty (wxT("fold.comment"), wxT("1") );
    StyledTextCtrl1->SetProperty (wxT("fold.compact"), wxT("1") );

    wxColor grey( 100, 100, 100 );
    StyledTextCtrl1->MarkerDefine (wxSTC_MARKNUM_FOLDER, wxSTC_MARK_ARROW );
    StyledTextCtrl1->MarkerSetForeground (wxSTC_MARKNUM_FOLDER, grey);
    StyledTextCtrl1->MarkerSetBackground (wxSTC_MARKNUM_FOLDER, grey);

    StyledTextCtrl1->MarkerDefine (wxSTC_MARKNUM_FOLDEROPEN,    wxSTC_MARK_ARROWDOWN);
    StyledTextCtrl1->MarkerSetForeground (wxSTC_MARKNUM_FOLDEROPEN, grey);
    StyledTextCtrl1->MarkerSetBackground (wxSTC_MARKNUM_FOLDEROPEN, grey);

    StyledTextCtrl1->MarkerDefine (wxSTC_MARKNUM_FOLDERSUB,     wxSTC_MARK_EMPTY);
    StyledTextCtrl1->MarkerSetForeground (wxSTC_MARKNUM_FOLDERSUB, grey);
    StyledTextCtrl1->MarkerSetBackground (wxSTC_MARKNUM_FOLDERSUB, grey);

    StyledTextCtrl1->MarkerDefine (wxSTC_MARKNUM_FOLDEREND,     wxSTC_MARK_ARROW);
    StyledTextCtrl1->MarkerSetForeground (wxSTC_MARKNUM_FOLDEREND, grey);
    StyledTextCtrl1->MarkerSetBackground (wxSTC_MARKNUM_FOLDEREND, _T("WHITE"));

    StyledTextCtrl1->MarkerDefine (wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN);
    StyledTextCtrl1->MarkerSetForeground (wxSTC_MARKNUM_FOLDEROPENMID, grey);
    StyledTextCtrl1->MarkerSetBackground (wxSTC_MARKNUM_FOLDEROPENMID, _T("WHITE"));

    StyledTextCtrl1->MarkerDefine (wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
    StyledTextCtrl1->MarkerSetForeground (wxSTC_MARKNUM_FOLDERMIDTAIL, grey);
    StyledTextCtrl1->MarkerSetBackground (wxSTC_MARKNUM_FOLDERMIDTAIL, grey);

    StyledTextCtrl1->MarkerDefine (wxSTC_MARKNUM_FOLDERTAIL,    wxSTC_MARK_EMPTY);
    StyledTextCtrl1->MarkerSetForeground (wxSTC_MARKNUM_FOLDERTAIL, grey);
    StyledTextCtrl1->MarkerSetBackground (wxSTC_MARKNUM_FOLDERTAIL, grey);

    StyledTextCtrl1->SetWrapMode (wxSTC_WRAP_WORD); // other choice is wxSCI_WRAP_NONE

    StyledTextCtrl1->StyleSetForeground (wxSTC_C_STRING,            wxColour(150,0,0));
    StyledTextCtrl1->StyleSetForeground (wxSTC_C_PREPROCESSOR,      wxColour(165,105,0));
    StyledTextCtrl1->StyleSetForeground (wxSTC_C_IDENTIFIER,        wxColour(40,0,60));
    StyledTextCtrl1->StyleSetForeground (wxSTC_C_NUMBER,            wxColour(0,150,0));
    StyledTextCtrl1->StyleSetForeground (wxSTC_C_CHARACTER,         wxColour(150,0,0));
    StyledTextCtrl1->StyleSetForeground (wxSTC_C_WORD,              wxColour(0,0,150));
    StyledTextCtrl1->StyleSetForeground (wxSTC_C_WORD2,             wxColour(0,150,0));
    StyledTextCtrl1->StyleSetForeground (wxSTC_C_COMMENT,           wxColour(150,150,150));
    StyledTextCtrl1->StyleSetForeground (wxSTC_C_COMMENTLINE,       wxColour(150,150,150));
    StyledTextCtrl1->StyleSetForeground (wxSTC_C_COMMENTDOC,        wxColour(150,150,150));
    StyledTextCtrl1->StyleSetForeground (wxSTC_C_COMMENTDOCKEYWORD, wxColour(0,0,200));
    StyledTextCtrl1->StyleSetForeground (wxSTC_C_COMMENTDOCKEYWORDERROR, wxColour(0,0,200));
    StyledTextCtrl1->StyleSetBold(wxSTC_C_WORD, true);
    StyledTextCtrl1->StyleSetBold(wxSTC_C_WORD2, true);
    StyledTextCtrl1->StyleSetBold(wxSTC_C_COMMENTDOCKEYWORD, true);

    StyledTextCtrl1->SetKeyWords(0, wxT("return for while break continue class public: private: protected:"));
    StyledTextCtrl1->SetKeyWords(1, wxT("const int float void char double"));

    wxMenuItem* SaveFile;
    SaveFile = new wxMenuItem(Menu1, idSaveFile, _("Save\tCtrl-S"), _("Save File"), wxITEM_NORMAL);
    Menu1->Append(SaveFile);

    Connect(idSaveFile,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&encIDEFrame::OnSave);

    wxMenuItem* OpenFile;
    OpenFile = new wxMenuItem(Menu1, idOpenFile, _("Open\tCtrl-O"), _("Open File"), wxITEM_NORMAL);
    Menu1->Append(OpenFile);

    Connect(idOpenFile,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&encIDEFrame::OnOpenFile);

    wxMenuItem* Compile;
    Compile = new wxMenuItem(Menu1, idComp, _("Compile\tCtrl-B"), _("Compile"), wxITEM_NORMAL);
    Menu1->Append(Compile);

    Connect(idComp,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&encIDEFrame::OnCompile);

    wxMenu *Menu3;
    Menu3 = new wxMenu();
    MenuBar1->Insert(1, Menu3, _("Other Options"));

    wxMenuItem* CompilerPath;
    CompilerPath = new wxMenuItem(Menu3, idCompPath, _("Set Compiler Path\tCtrl-Q"), _("Set Compiler Path"), wxITEM_NORMAL);
    Menu3->Append(CompilerPath);

    Connect(idCompPath,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&encIDEFrame::OnSetCompPath);

    wxMenuItem* RiscvTarget;
    RiscvTarget = new wxMenuItem(Menu3, idRiscvTarget, _("Set RISCV Target\tCtrl-T"), _("Set RISCV Target"), wxITEM_NORMAL);
    Menu3->Append(RiscvTarget);

    Connect(idRiscvTarget,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&encIDEFrame::OnSetRiscvTarget);

    wxMenuItem* RiscvPath;
    RiscvPath = new wxMenuItem(Menu3, idRiscvPath, _("Set RISCV Path\tCtrl-R"), _("Set RISCV Path"), wxITEM_NORMAL);
    Menu3->Append(RiscvPath);

    Connect(idRiscvPath,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&encIDEFrame::OnSetRiscvPath);

    wxMenuItem* Encrypt;
    Encrypt = new wxMenuItem(Menu3, idEncrypt, _("Encrypt\tCtrl-E"), _("Encrypt"), wxITEM_NORMAL);
    Menu3->Append(Encrypt);

    Connect(idEncrypt,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&encIDEFrame::OnEncrypt);

    wxMenuItem* CheckCompStr;
    CheckCompStr = new wxMenuItem(Menu3, idCheckCompStr, _("Check Compile String\tCtrl-K"), _("Check Compile String"), wxITEM_NORMAL);
    Menu3->Append(CheckCompStr);

    Connect(idCheckCompStr,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&encIDEFrame::OnCheckCompStr);

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(StyledTextCtrl1, 1, wxEXPAND);
    this->SetSizerAndFit(sizer);
}

encIDEFrame::~encIDEFrame()
{
    //(*Destroy(encIDEFrame)
    //*)
}

void encIDEFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void encIDEFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("encIDE", _("Welcome to..."));
}

void setCompileString(){
    wxGetApp().compStr = compPath + " " + riscvTarget + " " + riscvPath + " ";

    if(fileName != "")
        wxGetApp().compStr += " -c " + fileName + " -o " + replaceStr(fileName, ".c", ".o ");
}

void encIDEFrame::OnSave(wxCommandEvent& event)
{
    wxFileDialog fileDlg(this, "Save file...", "", "", "Save Files (*.c, *.cpp) | *.cpp | All files (*.*)|*.*", wxFD_SAVE);

    if (fileDlg.ShowModal() == wxID_OK)
    {
        wxFile file( fileDlg.GetPath() + ".c", wxFile::write /*read_write*/ );

        if( file.IsOpened() )
        {
            StyledTextCtrl1->SaveFile(fileDlg.GetPath() + ".c");

            fileName = fileDlg.GetPath().ToStdString() + ".c";
            setCompileString();

            file.Close();
        }
        else wxMessageBox("File cannot be created!");
    }
}






void encIDEFrame::OnOpenFile(wxCommandEvent& event)
{
    wxFileDialog fileDlg(this, "Open file...", "", "", "All files (*.*)|*.*", wxFD_OPEN);

    if (fileDlg.ShowModal() == wxID_OK)
    {

        wxTextFile tfile;
        tfile.Open(fileDlg.GetPath());

        if( tfile.IsOpened() )
        {

            StyledTextCtrl1->ClearAll();
            StyledTextCtrl1->AddText(tfile.GetFirstLine());
            StyledTextCtrl1->AddText("\r\n");
            while(!tfile.Eof())
            {
                StyledTextCtrl1->AddText(tfile.GetNextLine());
                StyledTextCtrl1->AddText("\r\n");
            }

            fileName = fileDlg.GetPath().ToStdString();
            setCompileString();

            tfile.Close();
        }
        else wxMessageBox("File cannot be opened!");
    }
}

void encIDEFrame::OnSetCompPath(wxCommandEvent& event)
{
    wxTextEntryDialog textDlg(this, "", "Give Compiler Path", "", wxOK | wxCANCEL, wxDefaultPosition);
    if(textDlg.ShowModal() == wxID_OK){
        compPath = textDlg.GetValue().ToStdString();
        setCompileString();
    }
}

void encIDEFrame::OnSetRiscvTarget(wxCommandEvent& event)
{
    wxTextEntryDialog textDlg(this, "", "Give RISCV Target", "", wxOK | wxCANCEL, wxDefaultPosition);
    if(textDlg.ShowModal() == wxID_OK){
        riscvTarget = textDlg.GetValue().ToStdString();
        setCompileString();
    }
}

void encIDEFrame::OnSetRiscvPath(wxCommandEvent& event)
{
    wxTextEntryDialog textDlg(this, "", "Give RISCV Path", "", wxOK | wxCANCEL, wxDefaultPosition);
    if(textDlg.ShowModal() == wxID_OK){
        riscvPath = textDlg.GetValue().ToStdString();
        setCompileString();
    }
}


void encIDEFrame::OnEncrypt(wxCommandEvent& event)
{
    encIDEenc* Frame = new encIDEenc(0);
    Frame->CenterOnParent();
    Frame->Show();

    this->Hide();
}

void encIDEFrame::OnCompile(wxCommandEvent& event)
{
    wxMessageBox(wxGetApp().compStr.c_str());
    system(wxGetApp().compStr.c_str());
}

void encIDEFrame::OnCheckCompStr(wxCommandEvent& event)
{
    wxTextEntryDialog textDlg(this, "", "Check Compiler String", wxGetApp().compStr, wxTE_MULTILINE|wxOK|wxCANCEL, wxDefaultPosition);
    if(textDlg.ShowModal() == wxID_OK)
        wxGetApp().compStr = textDlg.GetValue().ToStdString();
}
