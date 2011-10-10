/////////////////////////////////////////////////////////////////////////////
// Name:        wx/mac/carbon/palette.h
// Purpose:     wxPalette class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: palette.h 42752 2006-10-30 19:26:48Z VZ $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PALETTE_H_
#define _WX_PALETTE_H_

#include "wx/colour.h"
#include "wx/gdiobj.h"

class WXDLLEXPORT wxPalette;

class WXDLLEXPORT wxPaletteRefData: public wxGDIRefData
{
    DECLARE_NO_COPY_CLASS(wxPaletteRefData)

    friend class WXDLLEXPORT wxPalette;
public:
    wxPaletteRefData();
    virtual ~wxPaletteRefData();
protected:
    wxColour* m_palette;
    wxInt32   m_count ;
};

#define M_PALETTEDATA ((wxPaletteRefData *)m_refData)

class WXDLLEXPORT wxPalette: public wxPaletteBase
{
public:
    wxPalette();

    wxPalette(int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue);
    virtual ~wxPalette();
    bool Create(int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue);
    
    int GetPixel(unsigned char red, unsigned char green, unsigned char blue) const;
    bool GetRGB(int pixel, unsigned char *red, unsigned char *green, unsigned char *blue) const;

    virtual bool Ok() const { return IsOk(); }
    virtual bool IsOk() const { return (m_refData != NULL) ; }

    virtual int GetColoursCount() const;
    
private:
    DECLARE_DYNAMIC_CLASS(wxPalette)
};

#endif
    // _WX_PALETTE_H_
