#ifndef encIDEMAIN_H
#define encIDEMAIN_H

//(*Headers(encIDEFrame)
#include <wx/menu.h>
#include <wx/statusbr.h>
#include <wx/frame.h>
//*)

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/stc/stc.h>
#include <wx/sizer.h>
#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/textfile.h>
#include <wx/textdlg.h>
#include <wx/textctrl.h>
#include <wx/string.h>

#include "encIDEenc.h"

//std::string compStr;
class encIDEFrame: public wxFrame
{
    public:

        encIDEFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~encIDEFrame();

    private:

        //(*Handlers(encIDEFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnClose(wxCloseEvent& event);
        //*)

        void OnSave(wxCommandEvent& event);
        void OnOpenFile(wxCommandEvent& event);
        void OnCompile(wxCommandEvent& event);
        void OnSetCompPath(wxCommandEvent& event);
        void OnSetRiscvTarget(wxCommandEvent& event);
        void OnSetRiscvPath(wxCommandEvent& event);
        void OnEncrypt(wxCommandEvent& event);
        void OnCheckCompStr(wxCommandEvent& event);

        //(*Identifiers(encIDEFrame)
        static const long ID_MENUITEM1;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        static const long idSaveFile;
        static const long idOpenFile;
        static const long idComp;
        static const long idCompPath;
        static const long idRiscvTarget;
        static const long idRiscvPath;
        static const long idEncrypt;
        static const long idCheckCompStr;

        static const long ID_CHOICE1;
        static const long ID_CHOICE2;

        static const long ID_STYLEDTEXTCTRL1;



        //(*Declarations(encIDEFrame)
        wxStatusBar* StatusBar1;
        //*)

        wxChoice* Choice1;
        wxChoice* Choice2;

        wxStyledTextCtrl* StyledTextCtrl1;

        DECLARE_EVENT_TABLE()
};

#endif // encIDEMAIN_H
