#ifndef encIDEENC_H
#define encIDEENC_H

//(*Headers(encIDEenc)
#include <wx/checkbox.h>
#include <wx/frame.h>
//*)

#include <algorithm>
#include <vector>
#include <string>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/stattext.h>

#include <wx/event.h>
#include <wx/gdicmn.h>

#include "encIDEMain.h"
#include "encIDEselBits.h"

class encIDEenc: public wxFrame
{
	public:

		encIDEenc(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~encIDEenc();

		//(*Declarations(encIDEenc)
		//*)

		wxCheckBox* CHK_ienc32insts;
        wxCheckBox* CHK_menc32insts;
        wxCheckBox* CHK_aenc32insts;
        wxCheckBox* CHK_fenc32insts;
        wxCheckBox* CHK_denc32insts;
        wxCheckBox* CHK_qenc32insts;

        wxCheckBox* CHK_ienc64insts;
        wxCheckBox* CHK_menc64insts;
        wxCheckBox* CHK_aenc64insts;
        wxCheckBox* CHK_fenc64insts;
        wxCheckBox* CHK_denc64insts;
        wxCheckBox* CHK_qenc64insts;

        wxCheckBox* CHK_cencq2insts;
        wxCheckBox* CHK_cencq1insts;
        wxCheckBox* CHK_cencq0insts;


		wxButton* Button1;
		wxButton* goBckBtn;
		wxButton* addAlp;

		wxStaticText* LBL_ienc32insts;
        wxStaticText* LBL_menc32insts;
        wxStaticText* LBL_aenc32insts;
        wxStaticText* LBL_fenc32insts;
        wxStaticText* LBL_denc32insts;
        wxStaticText* LBL_qenc32insts;

        wxStaticText* LBL_ienc64insts;
        wxStaticText* LBL_menc64insts;
        wxStaticText* LBL_aenc64insts;
        wxStaticText* LBL_fenc64insts;
        wxStaticText* LBL_denc64insts;
        wxStaticText* LBL_qenc64insts;

        wxStaticText* LBL_cencq2insts;
        wxStaticText* LBL_cencq1insts;
        wxStaticText* LBL_cencq0insts;

        wxTextCtrl* key_text;
        wxTextCtrl* keyall_text;

		void OnButtons(wxCommandEvent& event);
		void OnCheckBoxes(wxCommandEvent& event);

	protected:

		//(*Identifiers(encIDEenc)
		//*)

		static const long ID_CHK_ienc32insts;
        static const long ID_CHK_menc32insts;
        static const long ID_CHK_aenc32insts;
        static const long ID_CHK_fenc32insts;
        static const long ID_CHK_denc32insts;
        static const long ID_CHK_qenc32insts;

        static const long ID_CHK_ienc64insts;
        static const long ID_CHK_menc64insts;
        static const long ID_CHK_aenc64insts;
        static const long ID_CHK_fenc64insts;
        static const long ID_CHK_denc64insts;
        static const long ID_CHK_qenc64insts;

        static const long ID_CHK_cencq2insts;
        static const long ID_CHK_cencq1insts;
        static const long ID_CHK_cencq0insts;

		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;

		static const long ID_LBL_ienc32insts;
        static const long ID_LBL_menc32insts;
        static const long ID_LBL_aenc32insts;
        static const long ID_LBL_fenc32insts;
        static const long ID_LBL_denc32insts;
        static const long ID_LBL_qenc32insts;

        static const long ID_LBL_ienc64insts;
        static const long ID_LBL_menc64insts;
        static const long ID_LBL_aenc64insts;
        static const long ID_LBL_fenc64insts;
        static const long ID_LBL_denc64insts;
        static const long ID_LBL_qenc64insts;

        static const long ID_LBL_cencq2insts;
        static const long ID_LBL_cencq1insts;
        static const long ID_LBL_cencq0insts;

        static const long ID_key_text;
        static const long ID_keyall_text;


	private:

		//(*Handlers(encIDEenc)
		//*)

		void OngoBckBtnClick(wxCommandEvent& event);
		void OnSelBitsBtnClick(wxCommandEvent& event);
		void OnaddAlpBtnClick(wxCommandEvent& event);
		void OnShow(wxShowEvent& event);


		DECLARE_EVENT_TABLE()
};


#endif
