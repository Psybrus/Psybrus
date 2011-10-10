/////////////////////////////////////////////////////////////////////////////
// Name:        wx/x11/cursor.h
// Purpose:     wxCursor class
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// RCS-ID:      $Id: cursor.h 42752 2006-10-30 19:26:48Z VZ $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CURSOR_H_
#define _WX_CURSOR_H_

#include "wx/bitmap.h"

#if wxUSE_IMAGE
#include "wx/image.h"
#endif

//-----------------------------------------------------------------------------
// wxCursor
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxCursor: public wxObject
{
public:

    wxCursor();
    wxCursor( int cursorId );
#if wxUSE_IMAGE
    wxCursor( const wxImage & image );
#endif
    wxCursor( const char bits[], int width, int  height,
              int hotSpotX=-1, int hotSpotY=-1,
              const char maskBits[]=0, wxColour *fg=0, wxColour *bg=0 );
    virtual ~wxCursor();
    bool Ok() const { return IsOk(); }
    bool IsOk() const;

    // implementation

    WXCursor GetCursor() const;

private:
    DECLARE_DYNAMIC_CLASS(wxCursor)
};


#endif
    // _WX_CURSOR_H_
