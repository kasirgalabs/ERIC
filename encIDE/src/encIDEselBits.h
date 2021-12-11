#ifndef encIDESELBITS_H
#define encIDESELBITS_H

//(*Headers(encIDEselBits)
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/choice.h>
//*)

#include <wx/button.h>
#include <wx/stattext.h>
#include "encIDEenc.h"

class encIDEselBits: public wxFrame
{
	public:

		encIDEselBits(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~encIDEselBits();

		//(*Declarations(encIDEselBits)
		wxStaticText* StaticText1;
		wxChoice* Choice1;
		//*)

		wxButton* goBckBtn;
		wxButton* addAlp;
		wxStaticText* lbl1;

		void OnBitButtons(wxCommandEvent& event);

	protected:

		//(*Identifiers(encIDEselBits)
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT1;
		//*)

		static const long ID_BUTTON1;
		static const long ID_BUTTON3;
		static const long ID_STATICTEXT2;

	private:

		//(*Handlers(encIDEselBits)
		void OnClose(wxCloseEvent& event);
		void OnChoice1Select(wxCommandEvent& event);
		//*)

		void OngoBckBtnClick(wxCommandEvent& event);
		void OnaddAlpBtnClick(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};

#endif
