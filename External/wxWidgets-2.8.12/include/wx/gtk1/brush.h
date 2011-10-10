/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk1/brush.h
// Purpose:
// Author:      Robert Roebling
// Id:          $Id: brush.h 41751 2006-10-08 21:56:55Z VZ $
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#ifndef __GTKBRUSHH__
#define __GTKBRUSHH__

#include "wx/defs.h"
#include "wx/object.h"
#include "wx/string.h"
#include "wx/gdiobj.h"
#include "wx/bitmap.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxBrush;

//-----------------------------------------------------------------------------
// wxBrush
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxBrush: public wxBrushBase
{
public:
    wxBrush() { }

    wxBrush( const wxColour &colour, int style = wxSOLID );
    wxBrush( const wxBitmap &stippleBitmap );
    virtual ~wxBrush();

    bool Ok() const { return IsOk(); }
    bool IsOk() const { return m_refData != NULL; }

    bool operator == ( const wxBrush& brush ) const;
    bool operator != (const wxBrush& brush) const { return !(*this == brush); }

    virtual int GetStyle() const;
    wxColour &GetColour() const;
    wxBitmap *GetStipple() const;

    void SetColour( const wxColour& col );
    void SetColour( unsigned char r, unsigned char g, unsigned char b );
    void SetStyle( int style );
    void SetStipple( const wxBitmap& stipple );

private:
    // ref counting code
    virtual wxObjectRefData *CreateRefData() const;
    virtual wxObjectRefData *CloneRefData(const wxObjectRefData *data) const;

    DECLARE_DYNAMIC_CLASS(wxBrush)
};

#endif // __GTKBRUSHH__
