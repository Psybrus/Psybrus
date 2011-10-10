/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dfb/app.h
// Purpose:     wxApp class
// Author:      Vaclav Slavik
// Created:     2006-08-10
// RCS-ID:      $Id: app.h 41133 2006-09-10 16:11:27Z VS $
// Copyright:   (c) 2006 REA Elektronik GmbH
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DFB_APP_H_
#define _WX_DFB_APP_H_

#include "wx/dfb/dfbptr.h"

#include "wx/vidmode.h"

wxDFB_DECLARE_INTERFACE(IDirectFB);

//-----------------------------------------------------------------------------
// wxApp
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxApp: public wxAppBase
{
public:
    wxApp();
    ~wxApp();

    // override base class (pure) virtuals
    virtual bool Initialize(int& argc, wxChar **argv);
    virtual void CleanUp();

    virtual void WakeUpIdle();
    virtual bool Yield(bool onlyIfNeeded = false);

    virtual wxVideoMode GetDisplayMode() const;
    virtual bool SetDisplayMode(const wxVideoMode& mode);

private:
    wxVideoMode m_videoMode;

    DECLARE_DYNAMIC_CLASS(wxApp)
    DECLARE_EVENT_TABLE()
};

#endif // _WX_DFB_APP_H_
