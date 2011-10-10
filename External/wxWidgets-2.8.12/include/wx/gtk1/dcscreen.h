/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk1/dcscreen.h
// Purpose:
// Author:      Robert Roebling
// Id:          $Id: dcscreen.h 37065 2006-01-23 02:28:01Z MR $
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __GTKDCSCREENH__
#define __GTKDCSCREENH__

#include "wx/dcclient.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxScreenDC;

//-----------------------------------------------------------------------------
// wxScreenDC
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxScreenDC : public wxPaintDC
{
public:
    wxScreenDC();
    virtual ~wxScreenDC();

    static bool StartDrawingOnTop( wxWindow *window );
    static bool StartDrawingOnTop( wxRect *rect = (wxRect *) NULL );
    static bool EndDrawingOnTop();

    // implementation

    static GdkWindow  *sm_overlayWindow;
    static int         sm_overlayWindowX;
    static int         sm_overlayWindowY;

protected:
    virtual void DoGetSize(int *width, int *height) const;

private:
    DECLARE_DYNAMIC_CLASS(wxScreenDC)
};

#endif

    // __GTKDCSCREENH__

