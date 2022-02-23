#include "encIDEApp.h"
#include "encIDEMain.h"

#include <wx/image.h>

bool encIDEApp::OnInit()
{
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if (wxsOK)
    {
    	encIDEFrame* mainFrame = new encIDEFrame(0);
    	mainFrame->Show();
    	SetTopWindow(mainFrame);
    }
    return wxsOK;
}

IMPLEMENT_APP(encIDEApp);
