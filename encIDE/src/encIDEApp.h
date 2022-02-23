#ifndef ENCIDEAPP_H
#define ENCIDEAPP_H

#include <wx/app.h>

class encIDEApp : public wxApp
{
    public:
        virtual bool OnInit();

        // Global string variables which are reachable within wxGetApp().stringName
        wxString compileString;
        wxString compileOptionsString;
};

DECLARE_APP(encIDEApp);

#endif
