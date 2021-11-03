/***************************************************************
 * Name:      kasirgaIDEApp.h
 * Purpose:   Defines Application Class
 * Author:     ()
 * Created:   2021-10-01
 * Copyright:  ()
 * License:
 **************************************************************/

#ifndef KASIRGAIDEAPP_H
#define KASIRGAIDEAPP_H

#include <wx/app.h>
#include <string.h>

class kasirgaIDEApp : public wxApp
{
    public:
        virtual bool OnInit();
        std::string compStr;
        std::string alpStr;
};

DECLARE_APP(kasirgaIDEApp);

#endif // KASIRGAIDEAPP_H
