/////////////////////////////////////////////////////////////////////////////
// Name:        minifram.h
// Purpose:     wxMiniFrame class. A small frame for e.g. floating toolbars.
//              If there is no equivalent on your platform, just make it a
//              normal frame.
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: minifram.h 41020 2006-09-05 20:47:48Z VZ $
// Copyright:   (c) Stefan Csomor
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
      Create(parent, id, title, pos, size, style | wxFRAME_TOOL_WINDOW | wxFRAME_FLOAT_ON_PARENT , name);
  }

  virtual ~wxMiniFrame() {}
protected:
};

#endif
    // _WX_MINIFRAM_H_
