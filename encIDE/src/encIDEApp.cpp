#include "encIDEApp.h"

#include "encIDEMain.h"
#include <wx/image.h>

IMPLEMENT_APP(encIDEApp);

bool encIDEApp::OnInit()
{
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	encIDEFrame* Frame = new encIDEFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    return wxsOK;

}
