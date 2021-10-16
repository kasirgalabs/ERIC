#ifndef KASIRGAIDESELBITS_H
#define KASIRGAIDESELBITS_H

//(*Headers(kasirgaIDEselBits)
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/choice.h>
//*)

#include <wx/button.h>
#include <wx/stattext.h>
#include "kasirgaIDEenc.h"

class kasirgaIDEselBits: public wxFrame
{
	public:

		kasirgaIDEselBits(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~kasirgaIDEselBits();

		//(*Declarations(kasirgaIDEselBits)
		wxStaticText* StaticText1;
		wxChoice* Choice1;
		//*)

		wxButton* goBckBtn;
		wxButton* addAlp;
		wxStaticText* lbl1;

		void OnBitButtons(wxCommandEvent& event);

	protected:

		//(*Identifiers(kasirgaIDEselBits)
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT1;
		//*)

		static const long ID_BUTTON1;
		static const long ID_BUTTON3;
		static const long ID_STATICTEXT2;

	private:

		//(*Handlers(kasirgaIDEselBits)
		void OnClose(wxCloseEvent& event);
		void OnChoice1Select(wxCommandEvent& event);
		//*)

		void OngoBckBtnClick(wxCommandEvent& event);
		void OnaddAlpBtnClick(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};

#endif
