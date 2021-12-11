#ifndef encIDEAPP_H
#define encIDEAPP_H

#include <wx/app.h>
#include <string.h>

class encIDEApp : public wxApp
{
    public:
        virtual bool OnInit();
        std::string compStr;
        std::string alpStr;
};

DECLARE_APP(encIDEApp);

#endif
