/////////////////////////////////////////////////////////////////////////////
// Name:        minifram.h
// Purpose:     wxMiniFrame class
// Author:      Robert Roebling
// RCS-ID:      $Id: minifram.h 42654 2006-10-29 20:15:26Z RR $
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __GTKMINIFRAMEH__
#define __GTKMINIFRAMEH__

#include "wx/defs.h"

#if wxUSE_MINIFRAME

#include "wx/object.h"
#include "wx/bitmap.h"
#include "wx/frame.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxMiniFrame;

//-----------------------------------------------------------------------------
// wxMiniFrame
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxMiniFrame: public wxFrame
{
    DECLARE_DYNAMIC_CLASS(wxMiniFrame)

public:
    wxMiniFrame() {}
    wxMiniFrame(wxWindow *parent,
            wxWindowID id,
            const wxString& title,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxCAPTION | wxRESIZE_BORDER,
            const wxString& name = wxFrameNameStr)
    {
        Create(parent, id, title, pos, size, style, name);
    }

    bool Create(wxWindow *parent,
            wxWindowID id,
            const wxString& title,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxCAPTION | wxRESIZE_BORDER,
            const wxString& name = wxFrameNameStr);

    virtual void SetTitle( const wxString &title );
 // implementation
 
    bool   m_isDragging;
    int    m_oldX,m_oldY;
    int    m_diffX,m_diffY;
    wxBitmap  m_closeButton;
};

#endif

#endif
  //  __GTKMINIFRAMEH__
