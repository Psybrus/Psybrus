/////////////////////////////////////////////////////////////////////////////
// Name:        wx/mac/carbon/cursor.h
// Purpose:     wxCursor class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: cursor.h 42752 2006-10-30 19:26:48Z VZ $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CURSOR_H_
#define _WX_CURSOR_H_

#include "wx/bitmap.h"

// Cursor
class WXDLLEXPORT wxCursor: public wxBitmap
{
  DECLARE_DYNAMIC_CLASS(wxCursor)

public:
  wxCursor();

  wxCursor(const char bits[], int width, int height, int hotSpotX = -1, int hotSpotY = -1,
    const char maskBits[] = NULL);

    wxCursor(const wxImage & image) ;
    wxCursor(const char **bits) ;
    wxCursor(char **bits) ;
    wxCursor(const wxString& name, long flags = wxBITMAP_TYPE_MACCURSOR_RESOURCE,
        int hotSpotX = 0, int hotSpotY = 0);

  wxCursor(int cursor_type);
  virtual ~wxCursor();

  bool CreateFromXpm(const char **bits) ;
  virtual bool Ok() const { return IsOk(); }
  virtual bool IsOk() const ;

    void MacInstall() const ;

  void SetHCURSOR(WXHCURSOR cursor);
  inline WXHCURSOR GetHCURSOR() const ;
private :
    void CreateFromImage(const wxImage & image) ;
};

extern WXDLLEXPORT void wxSetCursor(const wxCursor& cursor);

#endif
    // _WX_CURSOR_H_
