#include "encIDEApp.h"
#include "encIDEMain.h"

#include <wx/display.h>

#include <wx/msgdlg.h>

#include <wx/filedlg.h>
#include <wx/textfile.h>
#include <wx/textdlg.h>

#include <cstdlib>
#include <ctime>

#include "encIDEEnc.h"

#define DEFAULT_SCREEN_RATIO 0.8

#define TEXT_EDITOR_COLOR wxColor(180, 181, 185)
#define GRAY_COLOR wxColor(100, 100, 100)

#define MARGIN_LINE_NUMBERS 0
#define MARGIN_FOLD 1

#define CONFIG_FILE "encide.conf"

#define WINDOW_ANIMATION_RATIO 0.03
#define WINDOW_ANIMATION_MILLI_SECONDS 20

// File Menu Item IDs
const long encIDEFrame::idMenuOpenFile = wxNewId();
const long encIDEFrame::idMenuSaveFile = wxNewId();
const long encIDEFrame::idMenuCompileFile = wxNewId();
const long encIDEFrame::idMenuQuit = wxNewId();
// End of File Menu Item IDs

// View Menu Item IDs
const long encIDEFrame::idMenuZoomIn = wxNewId();
const long encIDEFrame::idMenuZoomOut = wxNewId();
// End of View Menu Item IDs

// Options Menu Item IDs
const long encIDEFrame::idMenuSetCompilerPath = wxNewId();
const long encIDEFrame::idMenuSetRiscvPath = wxNewId();
const long encIDEFrame::idMenuSetRiscvTargetOption = wxNewId();
const long encIDEFrame::idMenuAddExtraCompilerFlags = wxNewId();
const long encIDEFrame::idMenuCheckCompileString = wxNewId();
const long encIDEFrame::idMenuAddCustomEncryptor = wxNewId();
const long encIDEFrame::idMenuEncrypt = wxNewId();
// End of Options Menu Item IDs

// Help Menu Item IDs
const long encIDEFrame::idMenuAbout = wxNewId();
// End of Help Menu Item IDs

const long encIDEFrame::idStatusBar = wxNewId();

const long encIDEFrame::idTextEditor = wxNewId();


BEGIN_EVENT_TABLE(encIDEFrame, wxFrame)
    // File Menu Events
    EVT_MENU(idMenuOpenFile, encIDEFrame::onOpenFile)
    EVT_MENU(idMenuSaveFile, encIDEFrame::onSaveFile)
    EVT_MENU(idMenuCompileFile, encIDEFrame::onCompileFile)
    EVT_MENU(idMenuQuit, encIDEFrame::onQuit)
    // End of File Menu Events

    // View Menu Events
    EVT_MENU(idMenuZoomIn, encIDEFrame::onZoomIn)
    EVT_MENU(idMenuZoomOut, encIDEFrame::onZoomOut)
    // End of View Menu Events

    // Options Menu Events
    EVT_MENU(idMenuSetCompilerPath, encIDEFrame::onSetCompilerPath)
    EVT_MENU(idMenuSetRiscvPath, encIDEFrame::onSetRiscvPath)
    EVT_MENU(idMenuSetRiscvTargetOption, encIDEFrame::onSetRiscvTargetOption)
    EVT_MENU(idMenuAddExtraCompilerFlags, encIDEFrame::onAddExtraCompilerFlags)
    EVT_MENU(idMenuCheckCompileString, encIDEFrame::onCheckCompileString)
    EVT_MENU(idMenuAddCustomEncryptor, encIDEFrame::onAddCustomEncryptor)
    EVT_MENU(idMenuEncrypt, encIDEFrame::onEncrypt)
    // End of Options Menu Events

    // Help Menu Events
    EVT_MENU(idMenuAbout, encIDEFrame::onAbout)
    // End of Help Events
END_EVENT_TABLE()


encIDEFrame::encIDEFrame(wxWindow* parent, wxWindowID id)
{
    Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _("wxID_ANY"));

    wxGetApp().compileString = "";

    // Set variables
    filePath = "";
    compilerPath = "";
    riscvRootPath = "";
    riscvTargetOption = "";
    extraCompilerFlags = "";
    lastOpenedFile = "";

    // TODO Get custom-encryptor.h file location from cmake or config file
    customEncryptorPath = "/Users/shc/Downloads/objdump-main/ericyap/src/alp/custom-encryptor.h";
    
    screenRatio = DEFAULT_SCREEN_RATIO;

    windowWidth = -1;
    windowHeight = -1;

    windowPositionX = -1;
    windowPositionY = -1;

    zoomLevel = -1;

    // TODO set compile string from config file
    wxGetApp().compileString = "";
    wxGetApp().compileOptionsString = "";

    readConfigs();

    wxDisplay display(wxDisplay::GetFromWindow(this));
    wxRect activeScreenRect = display.GetClientArea();

    this->SetClientSize(windowWidth == -1 ? wxSize(screenRatio * activeScreenRect.GetWidth(), screenRatio * activeScreenRect.GetHeight()) : wxSize(windowWidth, windowHeight));

    if(windowPositionX == -1)
        this->CenterOnScreen();
    else
        this->SetPosition(wxPoint(windowPositionX, windowPositionY));

    // TODO make help descriptions more clear
    // TODO set keyboard shortcuts to seperate variables
    // MenuBar
    topMenuBar = new wxMenuBar();

    // TODO set menus within a function
    // File Menu
    fileMenu = new wxMenu();
    topMenuBar->Append(fileMenu, _("&File"));

    openFileItem = new wxMenuItem(fileMenu, idMenuOpenFile, _("Open File\tCtrl-O"), _("Open an existing file"), wxITEM_NORMAL);
    fileMenu->Append(openFileItem);

    saveFileItem = new wxMenuItem(fileMenu, idMenuSaveFile, _("Save File\tCtrl-S"), _("Save an existing or new file"), wxITEM_NORMAL);
    fileMenu->Append(saveFileItem);

    compileFileItem = new wxMenuItem(fileMenu, idMenuCompileFile, _("Compile File\tCtrl-B"), _("Execute compile string to compile existing file"), wxITEM_NORMAL);
    fileMenu->Append(compileFileItem);

    quitItem = new wxMenuItem(fileMenu, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    fileMenu->Append(quitItem);
    // End of File Menu

    // View Menu
    viewMenu = new wxMenu();
    topMenuBar->Append(viewMenu, _("&View"));

    zoomInItem = new wxMenuItem(viewMenu, idMenuZoomIn, _("Zoom In\tCtrl-+"), _("Zoom In"), wxITEM_NORMAL);
    zoomOutItem = new wxMenuItem(viewMenu, idMenuZoomOut, _("Zoom Out\tCtrl--"), _("Zoom Out"), wxITEM_NORMAL);

    // On macos set zoom key to 'option' or 'alt'
    #if defined(__APPLE__)
        zoomInItem->SetItemLabel(_("Zoom In\tAlt-+")); // Can be used with 'option' + 'shift' + '+'
        zoomOutItem->SetItemLabel(_("Zoom Out\tAlt--")); // Can be used with 'option' + '-'
    #endif

    viewMenu->Append(zoomInItem);
    viewMenu->Append(zoomOutItem);
    // End of View Menu

    // Options Menu
    optionsMenu = new wxMenu();
    topMenuBar->Append(optionsMenu, _("&Options"));

    setCompilerPathItem = new wxMenuItem(optionsMenu, idMenuSetCompilerPath, _("Set Compiler Path\tCtrl-P"), _("Set Compiler Path"), wxITEM_NORMAL);
    optionsMenu->Append(setCompilerPathItem);

    setRiscvPathItem = new wxMenuItem(optionsMenu, idMenuSetRiscvPath, _("Set RISCV Path\tCtrl-R"), _("Set RISCV Path"), wxITEM_NORMAL);
    optionsMenu->Append(setRiscvPathItem);

    setRiscvTargetOptionItem = new wxMenuItem(optionsMenu, idMenuSetRiscvTargetOption, _("Set RISCV Target Options\tCtrl-T"), _("Set RISCV Target Options"), wxITEM_NORMAL);
    optionsMenu->Append(setRiscvTargetOptionItem);

    addExtraCompilerFlagsItem = new wxMenuItem(optionsMenu, idMenuAddExtraCompilerFlags, _("Add Extra Compiler Flags\tCtrl-G"), _("Add Optional Compiler Flags"), wxITEM_NORMAL);
    optionsMenu->Append(addExtraCompilerFlagsItem);

    checkCompileStringItem = new wxMenuItem(optionsMenu, idMenuCheckCompileString, _("Check Compile String\tCtrl-K"), _("Check Compile String"), wxITEM_NORMAL);
    optionsMenu->Append(checkCompileStringItem);

    addCustomEncryptorItem = new wxMenuItem(optionsMenu, idMenuAddCustomEncryptor, _("Add Custom Encryptor\tCtrl-H"), _("Add custom encryptor to hex obfuscator"), wxITEM_NORMAL);
    optionsMenu->Append(addCustomEncryptorItem);

    encryptItem = new wxMenuItem(optionsMenu, idMenuEncrypt, _("Encrypt\tCtrl-E"), _("Encrypt"), wxITEM_NORMAL);
    optionsMenu->Append(encryptItem);
    // End of Options Menu

    // Help Menu
    helpMenu = new wxMenu();
    topMenuBar->Append(helpMenu, _("&Help"));

    aboutItem = new wxMenuItem(helpMenu, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    helpMenu->Append(aboutItem);
    // End of Help Menu

    SetMenuBar(topMenuBar);
    // End of MenuBar

    // Status Bar
    statusBar = new wxStatusBar(this, idStatusBar, wxSTB_DEFAULT_STYLE, _("StatusBar"));
    int __wxStatusBarWidths_1[1] = { -1 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    statusBar->SetFieldsCount(1, __wxStatusBarWidths_1);
    statusBar->SetStatusStyles(1, __wxStatusBarStyles_1);
    SetStatusBar(statusBar);
    // End of StatusBar

    // C++ Text Editor
    textEditor = new wxStyledTextCtrl(this, idTextEditor, wxDefaultPosition, wxDefaultSize, 0, "TextEditor");
    setTextEditorAndStyle();
    // End of C++ Text Editor
}

encIDEFrame::~encIDEFrame()
{
    whileAppClosing();
    this->Destroy();
}

void encIDEFrame::setTextEditorAndStyle(){
    textEditor->StyleSetBackground(wxSTC_STYLE_DEFAULT, TEXT_EDITOR_COLOR);

    // To not erase background color while writing
    textEditor->StyleClearAll();

    textEditor->SetLexer(wxSTC_LEX_CPP);

    // Set lines
    textEditor->SetMarginWidth(MARGIN_LINE_NUMBERS, 50);
    textEditor->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColor(75, 75, 75));
    textEditor->StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColor(220, 220, 220));
    textEditor->SetMarginType(MARGIN_LINE_NUMBERS, wxSTC_MARGIN_NUMBER);

    // Enable code folding
    textEditor->SetMarginType(MARGIN_FOLD, wxSTC_MARGIN_SYMBOL);
    textEditor->SetMarginWidth(MARGIN_FOLD, 1);
    textEditor->SetMarginMask(MARGIN_FOLD, wxSTC_MASK_FOLDERS);
    textEditor->StyleSetBackground(MARGIN_FOLD, wxColor(200, 200, 200));
    textEditor->SetMarginSensitive(MARGIN_FOLD, true);

    // Set scintilla properties
    textEditor->SetProperty(wxT("fold"), wxT("1"));
    textEditor->SetProperty(wxT("fold.comment"), wxT("1"));
    textEditor->SetProperty(wxT("fold.compact"), wxT("1"));

    // Set markers
    // Folder marker
    textEditor->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_ARROW);
    textEditor->MarkerSetForeground(wxSTC_MARKNUM_FOLDER, GRAY_COLOR);
    textEditor->MarkerSetBackground(wxSTC_MARKNUM_FOLDER, GRAY_COLOR);

    // Folder open marker
    textEditor->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_ARROWDOWN);
    textEditor->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, GRAY_COLOR);
    textEditor->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, GRAY_COLOR);

    // Folder sub marker
    textEditor->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY);
    textEditor->MarkerSetForeground(wxSTC_MARKNUM_FOLDERSUB, GRAY_COLOR);
    textEditor->MarkerSetBackground(wxSTC_MARKNUM_FOLDERSUB, GRAY_COLOR);

    // Folder end marker
    textEditor->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_ARROW);
    textEditor->MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, GRAY_COLOR);
    textEditor->MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, _("WHITE"));

    // Folder open mid marker
    textEditor->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN);
    textEditor->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, GRAY_COLOR);
    textEditor->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, _("WHITE"));

    // Folder mid tail marker
    textEditor->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
    textEditor->MarkerSetForeground(wxSTC_MARKNUM_FOLDERMIDTAIL, GRAY_COLOR);
    textEditor->MarkerSetBackground(wxSTC_MARKNUM_FOLDERMIDTAIL, GRAY_COLOR);

    // Folder tail marker
    textEditor->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY);
    textEditor->MarkerSetForeground(wxSTC_MARKNUM_FOLDERTAIL, GRAY_COLOR);
    textEditor->MarkerSetBackground(wxSTC_MARKNUM_FOLDERTAIL, GRAY_COLOR);

    // Set wrap mode, other choice can be wxSCI_WRAP_NONE 
    textEditor->SetWrapMode(wxSTC_WRAP_WORD);

    // Set other style options and colors
    textEditor->StyleSetForeground(wxSTC_C_STRING, wxColor(150, 0, 0));
    textEditor->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColor(165, 105, 0));
    textEditor->StyleSetForeground(wxSTC_C_IDENTIFIER, wxColor(40, 0, 60));
    textEditor->StyleSetForeground(wxSTC_C_NUMBER, wxColor(0, 150, 0));
    textEditor->StyleSetForeground(wxSTC_C_CHARACTER, wxColor(150, 0, 0));

    textEditor->StyleSetForeground(wxSTC_C_WORD, wxColor(0, 0, 150));
    textEditor->StyleSetForeground(wxSTC_C_WORD2, wxColor(0, 150, 0));

    textEditor->StyleSetForeground(wxSTC_C_COMMENT, wxColor(150, 150, 150));
    textEditor->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColor(150, 150, 150));
    textEditor->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColor(150, 150, 150));

    textEditor->StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORD, wxColor(0, 0, 200));
    textEditor->StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORDERROR, wxColor(0, 0, 200));

    textEditor->StyleSetBold(wxSTC_C_WORD, true);
    textEditor->StyleSetBold(wxSTC_C_WORD2, true);
    textEditor->StyleSetBold(wxSTC_C_COMMENTDOCKEYWORD, true);

    // TODO add more c++ keyword
    // c++ keywords
    textEditor->SetKeyWords(0, wxT("return for while break continue class public private protected"));
    textEditor->SetKeyWords(1, wxT("const void int float char double"));

    if(zoomLevel != -1)
        textEditor->SetZoom(zoomLevel);

    if(lastOpenedFile != wxEmptyString){
        wxTextFile openTextFile;
        openTextFile.Open(lastOpenedFile);
        if(openTextFile.IsOpened()){
            filePath = lastOpenedFile;

            textEditor->ClearAll();

            textEditor->AddText(openTextFile.GetFirstLine());
            textEditor->AddText("\r\n");
            while(!openTextFile.Eof()){
                textEditor->AddText(openTextFile.GetNextLine());
                textEditor->AddText("\r\n");
            }

            statusBar->PushStatusText(filePath);

            openTextFile.Close();
        }
        else wxMessageBox("Last opened file cannot found!");
    }
}

void encIDEFrame::onOpenFile(wxCommandEvent& event)
{
    wxFileDialog openFileDlg(this, "Open file...", wxEmptyString, wxEmptyString, "All files (*.*)|*.*", wxFD_OPEN);

    if(openFileDlg.ShowModal() == wxID_OK){
        wxTextFile openTextFile;
        openTextFile.Open(openFileDlg.GetPath());

        if(openTextFile.IsOpened()){
            filePath = openFileDlg.GetPath();

            textEditor->ClearAll();

            textEditor->AddText(openTextFile.GetFirstLine());
            textEditor->AddText("\r\n");
            while(!openTextFile.Eof()){
                textEditor->AddText(openTextFile.GetNextLine());
                textEditor->AddText("\r\n");
            }

            statusBar->PushStatusText(filePath);

            openTextFile.Close();
        }
        else wxMessageBox("File cannot be opened!");
    }
}

// TODO Add new file support and distinguish between saving new file and saving existing file
void encIDEFrame::onSaveFile(wxCommandEvent& event)
{
    // Save existing file with same name
    wxFileDialog saveFileDlg(this, "Save file...", wxEmptyString, (filePath == wxEmptyString) ? _(wxEmptyString):filePath.substr(filePath.rfind('/')).erase(0, 1), "All files (*.*)|*.*", wxFD_SAVE);

    if(saveFileDlg.ShowModal() == wxID_OK){
        wxTextFile saveTextFile;
        saveTextFile.Create(saveFileDlg.GetPath());
        saveTextFile.Open(saveFileDlg.GetPath());

        if(saveTextFile.IsOpened()){
            filePath = saveFileDlg.GetPath();

            textEditor->SaveFile(filePath);

            statusBar->PushStatusText(filePath);

            saveTextFile.Close();
        }
        else wxMessageBox("File cannot be created!");
    }
}

void encIDEFrame::onCompileFile(wxCommandEvent& event)
{
    wxString tempFilePath = filePath;

    if(filePath != ""){
        #if defined(_WIN32)
            wxString foroFile = filePath.substr(filePath.rfind('\\')).erase(0, 1).erase(filePath.substr(filePath.rfind('\\')).erase(0, 1).rfind('.')) + ".o";
            wxGetApp().compileString = compilerPath + " -c " + extraCompilerFlags + " " + riscvTargetOption + " " + riscvRootPath + " " + filePath + " -o " + filePath.erase(filePath.rfind('\\')) + "\\" + foroFile;
        #else
            wxString foroFile = filePath.substr(filePath.rfind('/')).erase(0, 1).erase(filePath.substr(filePath.rfind('/')).erase(0, 1).rfind('.')) + ".o";
            wxGetApp().compileString = compilerPath + " -c " + extraCompilerFlags + " " + riscvTargetOption + " " + riscvRootPath + " " + filePath + " -o " + filePath.erase(filePath.rfind('/')) + "/" + foroFile;
        #endif
    }

    #if defined(_WIN32)
        wxExecute("start cmd.exe /k \"" + wxGetApp().compileString) + "\"";
    #elif defined(__APPLE__)
        wxExecute(" osascript -e 'tell application \"Terminal\" to activate' -e 'tell application \"Terminal\" to do script \"" + wxGetApp().compileString + " " + "\"'");    
    #else
    // every linux distribution has xterm?
        wxExecute("xterm -e \"" + wxGetApp().compileString + ";bash\"");
    #endif

    filePath = tempFilePath;

    // TODO make a decision between open command prompt or show result of execution that fails or not

}

void encIDEFrame::onQuit(wxCommandEvent& event)
{
    this->Close();
}

void encIDEFrame::onZoomIn(wxCommandEvent& event)
{
    textEditor->ZoomIn();
}

void encIDEFrame::onZoomOut(wxCommandEvent& event)
{
    textEditor->ZoomOut();
}

void encIDEFrame::onSetCompilerPath(wxCommandEvent& event)
{
    wxTextEntryDialog setCompilerPathDlg(this, wxEmptyString, "Set Compiler Path", 
        compilerPath/*TODO add compiler path from config file*/, wxOK | wxCANCEL | wxCENTRE, wxDefaultPosition);
    
    if(setCompilerPathDlg.ShowModal() == wxID_OK)
        compilerPath = setCompilerPathDlg.GetValue();
}

void encIDEFrame::onSetRiscvPath(wxCommandEvent& event)
{
    wxTextEntryDialog setRiscvPathDlg(this, wxEmptyString, "Set RISCV Path", 
        riscvRootPath /*TODO add riscv path from config file*/, wxOK | wxCANCEL | wxCENTRE, wxDefaultPosition);
    
    if(setRiscvPathDlg.ShowModal() == wxID_OK)
        riscvRootPath = setRiscvPathDlg.GetValue();
}

void encIDEFrame::onSetRiscvTargetOption(wxCommandEvent& event)
{
    wxTextEntryDialog setRiscvTargetOptionDlg(this, wxEmptyString, "Set RISCV Target Options", 
        riscvTargetOption /*TODO add riscv target options from config file*/, wxOK | wxCANCEL | wxCENTRE, wxDefaultPosition);
    
    if(setRiscvTargetOptionDlg.ShowModal() == wxID_OK)
        riscvTargetOption = setRiscvTargetOptionDlg.GetValue();
}

void encIDEFrame::onAddExtraCompilerFlags(wxCommandEvent& event)
{
    wxTextEntryDialog addExtraCompilerFlagsDlg(this, wxEmptyString, "Add Extra Compiler Flags", 
        extraCompilerFlags, wxTE_MULTILINE | wxOK | wxCANCEL | wxCENTRE, wxDefaultPosition);
    
    if(addExtraCompilerFlagsDlg.ShowModal() == wxID_OK)
        extraCompilerFlags = addExtraCompilerFlagsDlg.GetValue();
}

void encIDEFrame::onCheckCompileString(wxCommandEvent& event)
{
    wxString tempFilePath = filePath;

    if(filePath != ""){
        #if defined(_WIN32)
            wxString foroFile = filePath.substr(filePath.rfind('\\')).erase(0, 1).erase(filePath.substr(filePath.rfind('\\')).erase(0, 1).rfind('.')) + ".o";
            wxGetApp().compileString = compilerPath + " -c " + extraCompilerFlags + " " + riscvTargetOption + " " + riscvRootPath + " " + filePath + " -o " + filePath.erase(filePath.rfind('\\')) + "\\" + foroFile;
        #else
            wxString foroFile = filePath.substr(filePath.rfind('/')).erase(0, 1).erase(filePath.substr(filePath.rfind('/')).erase(0, 1).rfind('.')) + ".o";
            wxGetApp().compileString = compilerPath + " -c " + extraCompilerFlags + " " + riscvTargetOption + " " + riscvRootPath + " " + filePath + " -o " + filePath.erase(filePath.rfind('/')) + "/" + foroFile;
        #endif
    }

    filePath = tempFilePath;

    wxTextEntryDialog checkCompileStringDlg(this, wxEmptyString, "Check Compile String", 
        wxGetApp().compileString, wxTE_MULTILINE | wxOK | wxCANCEL | wxCENTRE, wxDefaultPosition);
    
    if(checkCompileStringDlg.ShowModal() == wxID_OK)
        wxGetApp().compileString = checkCompileStringDlg.GetValue();
}

// TODO add cancelling custom encryptor addition
// (maybe when a transition happened to another file, popstatustext, catch popping and cancel)
void encIDEFrame::onAddCustomEncryptor(wxCommandEvent& event)
{
    // When selected Add Custom Encryptor
    // TODO add more robust control
    if(addCustomEncryptorItem->GetHelp() == "Add custom encryptor to hex obfuscator"){
        // TODO make a decision here to get default text from customEncryptorPath or set default textEditor
        // TODO make a decision to add config file whether custom encryptor added before or not

        //textEditor->SetText();

        // /Users/shc/Downloads/objdump-main/ericyap/build

        wxTextFile openTextFile;
        openTextFile.Open(customEncryptorPath);

        if(openTextFile.IsOpened()){
            textEditor->ClearAll();

            textEditor->AddText(openTextFile.GetFirstLine());
            textEditor->AddText("\r\n");
            while(!openTextFile.Eof()){
                textEditor->AddText(openTextFile.GetNextLine());
                textEditor->AddText("\r\n");
            }

            openTextFile.Close();
        }
        else wxMessageBox("Custom Encryptor File cannot be opened!");

        statusBar->PushStatusText("Add custom encryptor to --> " + customEncryptorPath);

        addCustomEncryptorItem->SetItemLabel(_("Push Custom Encryptor\tCtrl-J"));
        addCustomEncryptorItem->SetHelp("Push custom encryptor and build the compiler and hex obfuscator");
    }
    else{
        // TODO add redirect textEditor to customEncryptorPath execution
        // TODO add custom encryptor cmake build execution (cmake --build ericsbuildpath)
        textEditor->SaveFile(customEncryptorPath);

        //wxMessageBox("CUSTOM ENCRYPTOR PUSHED!");

        #if defined(_WIN32)
            wxString tempStr = compilerPath;
            wxString cmakeBuildPath = compilerPath.erase(compilerPath.rfind('\\'));
            cmakeBuildPath.erase(cmakeBuildPath.rfind('\\'));
            compilerPath = tempStr; 
        #else
            wxString tempStr = compilerPath;
            wxString cmakeBuildPath = compilerPath.erase(compilerPath.rfind('/'));
            cmakeBuildPath.erase(cmakeBuildPath.rfind('/'));
            compilerPath = tempStr; 
        #endif

        #if defined(_WIN32)
            wxExecute("start cmd.exe /k \"cd " + cmakeBuildPath + ";cmake --build .\"";
        #elif defined(__APPLE__)
            wxExecute("osascript -e 'tell application \"Terminal\" to activate' -e 'tell application \"Terminal\" to do script \" cd " + cmakeBuildPath + ";cmake --build .\"'");
        #else
        // every linux distribution has xterm?
            wxExecute("xterm -e \"cd " + cmakeBuildPath + ";cmake --build .;bash\"");
        #endif


        statusBar->PushStatusText("");

        addCustomEncryptorItem->SetItemLabel(_("Add Custom Encryptor\tCtrl-H"));
        addCustomEncryptorItem->SetHelp("Add custom encryptor to hex obfuscator");
    }
}

void encIDEFrame::onEncrypt(wxCommandEvent& event)
{
    encIDEEnc* encFrame = new encIDEEnc(0);
    encFrame->CenterOnParent();
    encFrame->Show();

    this->Hide();
}

void encIDEFrame::onAbout(wxCommandEvent& event)
{
    wxMessageBox(wxVERSION_STRING, _("encIDE"));
}

// Helper function
wxString getSubStrAfter(wxString str, wxString delimiter){
    return str.substr(str.find(delimiter) + 1);
}

void encIDEFrame::readConfigs()
{
    wxTextFile configFile;
    configFile.Open(CONFIG_FILE);

    // TODO Fix corresponding strings
    if(configFile.IsOpened()){
        // first line is compiler path
        compilerPath = getSubStrAfter(configFile.GetFirstLine(), "=");

        // second line is riscv root path
        riscvRootPath = getSubStrAfter(configFile.GetNextLine(), "=");

        // third line is riscv target option
        riscvTargetOption = getSubStrAfter(configFile.GetNextLine(), "=");

        // fourth line is extra compiler flags
        extraCompilerFlags = getSubStrAfter(configFile.GetNextLine(), "=");

        // fifth line is last opened file
        lastOpenedFile = getSubStrAfter(configFile.GetNextLine(), "=");

        // sixth line is screen ratio
        getSubStrAfter(configFile.GetNextLine(), "=").ToDouble(&screenRatio);

        // seventh line is window width
        wxString wdW = getSubStrAfter(configFile.GetNextLine(), "=");
        if(wdW != wxEmptyString)
            wdW.ToLong(&windowWidth);
        
        // eighth line is window height
        wxString wdH = getSubStrAfter(configFile.GetNextLine(), "=");
        if(wdH != wxEmptyString)
            wdH.ToLong(&windowHeight);

        // ninth line is window x position
        wxString wdPX = getSubStrAfter(configFile.GetNextLine(), "=");
        if(wdPX != wxEmptyString)
            wdPX.ToLong(&windowPositionX);
        
        // tenth line is window y position
        wxString wdPY = getSubStrAfter(configFile.GetNextLine(), "=");
        if(wdPY != wxEmptyString)
            wdPY.ToLong(&windowPositionY);

        // eleventh line is text editor zoom level
        wxString zL = getSubStrAfter(configFile.GetNextLine(), "=");
        if(zL != wxEmptyString)
            zL.ToLong(&zoomLevel);

        configFile.Close();
    }
}

void encIDEFrame::resetConfigs()
{
    wxTextFile configFile;
    configFile.Open(CONFIG_FILE);

    // TODO Fix corresponding strings
    if(configFile.IsOpened()){
        configFile.Clear();

        // first line is compiler path
        configFile.AddLine("compilerpath=" + compilerPath);

        // second line is riscv root path
        configFile.AddLine("riscvrootpath=" + riscvRootPath);

        // third line is riscv target option
        configFile.AddLine("riscvtargetoption=" + riscvTargetOption);

        // fourth line is extra compiler flags
        configFile.AddLine("extracompilerflags=" + extraCompilerFlags);

        // fifth line is last opened file
        configFile.AddLine("lastopenedfile=" + lastOpenedFile);

        // sixth line is screen ratio
        configFile.AddLine("screenratio=" + wxString::Format(wxT("%lf"), screenRatio));

        // seventh line is window width
        configFile.AddLine("windowwidth=" + wxString::Format(wxT("%i"), (int)windowWidth));
        
        // eighth line is window height
        configFile.AddLine("windowheight=" + wxString::Format(wxT("%i"), (int)windowHeight));

        // ninth line is window x position
        configFile.AddLine("windowpositionx=" + wxString::Format(wxT("%i"), (int)windowPositionX));

        // tenth line is window y position
        configFile.AddLine("windowpositiony=" + wxString::Format(wxT("%i"), (int)windowPositionY));

        // eleventh line is text editor zoom level
        configFile.AddLine("zoomlevel=" + wxString::Format(wxT("%i"), (int)zoomLevel));

        // Write added lines to file
        configFile.Write();
        configFile.Close();
    }
}

void encIDEFrame::whileAppClosing(){
    if(filePath != wxEmptyString)
        lastOpenedFile = filePath;
    
    windowWidth = this->GetSize().GetWidth();

    // this->GetSize().GetHeight() returns height including statusbar
    // However SetClientSize does not include statusbar height
    // Because statusbar added after setting client size
    // That's why statusbar height should be subtracted
    // Another and maybe better solution can be using SetClientSize after adding statusbar in the constructor
    windowHeight = this->GetSize().GetHeight() - statusBar->GetSize().GetHeight();

    // Top left x-y coordinates
    windowPositionX = this->GetPosition().x;
    windowPositionY = this->GetPosition().y;

    zoomLevel = textEditor->GetZoom();

    resetConfigs();

    animateWindowWhileClosing();
}

// TODO make here more configurable
void encIDEFrame::animateWindowWhileClosing(){
    srand(time(NULL));
    bool randNum = (bool)(rand() % 2);

    if(randNum == false){
        double wW = 0;
        double wH = 0;

        for(int i=0; wW < windowWidth; i++){
            wW += windowWidth * WINDOW_ANIMATION_RATIO * i;
            wH += windowHeight * WINDOW_ANIMATION_RATIO * i;

            SetSize(wxSize((int)(windowWidth - wW), (int)(windowHeight - wW)));

            textEditor->SetZoom(zoomLevel - 5 * i);

            wxYield();
            wxMilliSleep(WINDOW_ANIMATION_MILLI_SECONDS);
        }
    }
    else{
        // TODO make these member variables (using also in constructor) if we will use this animation
        wxDisplay display(wxDisplay::GetFromWindow(this));
        wxRect activeScreenRect = display.GetClientArea();

        SetPosition(wxPoint(windowPositionX + activeScreenRect.GetWidth()*0.4, windowPositionY));
        wxYield();
        wxMilliSleep(300);

        SetPosition(wxPoint(windowPositionX + activeScreenRect.GetWidth()*0.4, windowPositionY + activeScreenRect.GetHeight()*0.4));
        wxYield();
        wxMilliSleep(300);

        SetPosition(wxPoint(windowPositionX, windowPositionY + activeScreenRect.GetHeight()*0.4));
        wxYield();
        wxMilliSleep(300);

        SetPosition(wxPoint(windowPositionX, windowPositionY));
        wxYield();
        wxMilliSleep(300);

        double wW = 0;
        double wH = 0;

        for(int i=0; wW < windowWidth; i++){
            wW += windowWidth * WINDOW_ANIMATION_RATIO * i;
            wH += windowHeight * WINDOW_ANIMATION_RATIO * i;

            SetSize(wxSize((int)(windowWidth - wW), (int)(windowHeight - wW)));

            textEditor->SetZoom(zoomLevel - 5 * i);

            wxYield();
            wxMilliSleep(WINDOW_ANIMATION_MILLI_SECONDS);
        }
    }
}
