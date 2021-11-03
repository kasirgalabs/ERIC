#ifndef KASIRGAIDEENC_H
#define KASIRGAIDEENC_H

//(*Headers(kasirgaIDEenc)
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

#include "kasirgaIDEMain.h"
#include "kasirgaIDEselBits.h"

class kasirgaIDEenc: public wxFrame
{
	public:

		kasirgaIDEenc(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~kasirgaIDEenc();

		//(*Declarations(kasirgaIDEenc)
		wxCheckBox* CheckBox1;
		wxCheckBox* CheckBox4;
		wxCheckBox* CheckBox2;
		wxCheckBox* CheckBox3;
		wxCheckBox* CheckBox5;
		//*)

		wxButton* Button1;
		wxButton* goBckBtn;
		wxButton* addAlp;
		//wxStaticText* lbl1;
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

		void OnButtons(wxCommandEvent& event);
		void OnCheckBoxes(wxCommandEvent& event);

	protected:

		//(*Identifiers(kasirgaIDEenc)
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX5;
		//*)

		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		//static const long ID_STATICTEXT1;
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


	private:

		//(*Handlers(kasirgaIDEenc)
		//*)

		void OngoBckBtnClick(wxCommandEvent& event);
		void OnSelBitsBtnClick(wxCommandEvent& event);
		void OnaddAlpBtnClick(wxCommandEvent& event);
		void OnShow(wxShowEvent& event);


		DECLARE_EVENT_TABLE()
};




#endif
