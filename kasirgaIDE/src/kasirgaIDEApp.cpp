/***************************************************************
 * Name:      kasirgaIDEApp.cpp
 * Purpose:   Code for Application Class
 * Author:     ()
 * Created:   2021-10-01
 * Copyright:  ()
 * License:
 **************************************************************/

#include "kasirgaIDEApp.h"

//(*AppHeaders
#include "kasirgaIDEMain.h"
#include <wx/image.h>
//*)
//#ifdef _DEBUG
//wxIMPLEMENT_APP_CONSOLE(kasirgaIDEApp);
//#else
//wxIMPLEMENT_APP(kasirgaIDEApp);
//#endif
IMPLEMENT_APP(kasirgaIDEApp);

bool kasirgaIDEApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	kasirgaIDEFrame* Frame = new kasirgaIDEFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

//kasirgaIDEApp::compStr = "";
