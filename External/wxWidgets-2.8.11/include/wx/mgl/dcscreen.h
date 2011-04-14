/////////////////////////////////////////////////////////////////////////////
// Name:        dcscreen.h
// Purpose:
// Author:      Vaclav Slavik
// Id:          $Id: dcscreen.h 41020 2006-09-05 20:47:48Z VZ $
// Copyright:   (c) 2001-2002 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_DCSCREEN_H__
#define __WX_DCSCREEN_H__

#include "wx/dcclient.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class WXDLLEXPORT wxScreenDC;

//-----------------------------------------------------------------------------
// wxScreenDC
//-----------------------------------------------------------------------------

class WXDLLEXPORT wxScreenDC: public wxDC
{
public:
    wxScreenDC();
    virtual ~wxScreenDC();

    static bool StartDrawingOnTop(wxWindow *WXUNUSED(window)) { return TRUE; }
    static bool StartDrawingOnTop(wxRect *WXUNUSED(rect) = NULL) { return TRUE; }
    static bool EndDrawingOnTop() { return TRUE; }
  
private:
    DECLARE_DYNAMIC_CLASS(wxScreenDC)
};

#endif

    // __WX_DCSCREEN_H__

