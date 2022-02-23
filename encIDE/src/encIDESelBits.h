#ifndef ENCIDESELBITS_H
#define ENCIDESELBITS_H

#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/choice.h>

#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/combobox.h>

#include <wx/textctrl.h>

#include "encIDEEnc.h"

class encIDESelBits: public wxFrame
{
	public:

		encIDESelBits(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~encIDESelBits();

		wxStaticText* StaticText1;
		//wxChoice* Choice1;
		wxComboBox* Choice1;

		wxButton* goBckBtn;
		wxButton* addAlp;
		wxStaticText* lbl1;
		wxStaticText *instTypeLbl;

		void OnBitButtons(wxCommandEvent& event);
		void OnBlockButtons(wxCommandEvent& event);
		std::string CreateDynamicButtons(std::string inst);

	protected:

		static const long ID_CHOICE1;
		static const long ID_STATICTEXT1;

		static const long ID_BUTTON1;
		static const long ID_BUTTON3;
		static const long ID_STATICTEXT2;

	private:

		void OnClose(wxCloseEvent& event);
		void OnChoice1Select(wxCommandEvent& event);

		void OnChoiceTextUpdated(wxCommandEvent& event);

		void OngoBckBtnClick(wxCommandEvent& event);
		void OnaddAlpBtnClick(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};

#endif
