/////////////////////////////////////////////////////////////////////////////
// Name:        wx/palmos/brush.h
// Purpose:     wxBrush class
// Author:      William Osborne - minimal working wxPalmOS port
// Modified by:
// Created:     10/13/04
// RCS-ID:      $Id: brush.h 41751 2006-10-08 21:56:55Z VZ $
// Copyright:   (c) William Osborne
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BRUSH_H_
#define _WX_BRUSH_H_

#include "wx/gdicmn.h"
#include "wx/gdiobj.h"
#include "wx/bitmap.h"

class WXDLLEXPORT wxBrush;

// ----------------------------------------------------------------------------
// wxBrush
// ----------------------------------------------------------------------------

class WXDLLEXPORT wxBrush : public wxBrushBase
{
public:
    wxBrush();
    wxBrush(const wxColour& col, int style = wxSOLID);
    wxBrush(const wxBitmap& stipple);
    virtual ~wxBrush();

    virtual void SetColour(const wxColour& col);
    virtual void SetColour(unsigned char r, unsigned char g, unsigned char b);
    virtual void SetStyle(int style);
    virtual void SetStipple(const wxBitmap& stipple);

    bool operator==(const wxBrush& brush) const;
    bool operator!=(const wxBrush& brush) const { return !(*this == brush); }

    wxColour GetColour() const;
    virtual int GetStyle() const;
    wxBitmap *GetStipple() const;

    bool Ok() const { return IsOk(); }
    bool IsOk() const { return m_refData != NULL; }

    // return the HBRUSH for this brush
    virtual WXHANDLE GetResourceHandle() const;

protected:
    virtual wxObjectRefData *CreateRefData() const;
    virtual wxObjectRefData *CloneRefData(const wxObjectRefData *data) const;

private:
    DECLARE_DYNAMIC_CLASS(wxBrush)
};

#endif
    // _WX_BRUSH_H_
