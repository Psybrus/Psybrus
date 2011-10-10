/////////////////////////////////////////////////////////////////////////////
// Name:        wx/x11/minifram.h
// Purpose:     wxMiniFrame class. A small frame for e.g. floating toolbars.
//              If there is no equivalent on your platform, just make it a
//              normal frame.
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// RCS-ID:      $Id: minifram.h 42077 2006-10-17 14:44:52Z ABX $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MINIFRAM_H_
#define _WX_MINIFRAM_H_

#include "wx/frame.h"

class WXDLLEXPORT wxMiniFrame: public wxFrame {

    DECLARE_DYNAMIC_CLASS(wxMiniFrame)

public:
    inline wxMiniFrame() {}
    inline wxMiniFrame(wxWindow *parent,
        wxWindowID id,
        const wxString& title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE|wxTINY_CAPTION_HORIZ,
        const wxString& name = wxFrameNameStr)
    {
        // Use wxFrame constructor in absence of more specific code.
        Create(parent, id, title, pos, size, style, name);
    }

    virtual ~wxMiniFrame() {}
protected:
};

#endif
    // _WX_MINIFRAM_H_
