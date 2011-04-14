/////////////////////////////////////////////////////////////////////////////
// Name:        wx/x11/palette.h
// Purpose:     wxPalette class
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// RCS-ID:      $Id: palette.h 42752 2006-10-30 19:26:48Z VZ $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PALETTE_H_
#define _WX_PALETTE_H_

#include "wx/gdiobj.h"
#include "wx/list.h"

class WXDLLEXPORT wxPalette;

// Palette for one display
class wxXPalette : public wxObject
{
    DECLARE_DYNAMIC_CLASS(wxXPalette)

public:
    wxXPalette();

    WXDisplay*        m_display;
    int               m_pix_array_n;
    unsigned long*    m_pix_array;
    WXColormap        m_cmap;
    bool              m_destroyable;
};

class WXDLLEXPORT wxPaletteRefData: public wxGDIRefData
{
    friend class WXDLLEXPORT wxPalette;
public:
    wxPaletteRefData();
    virtual ~wxPaletteRefData();

protected:
    wxList  m_palettes;
};

#define M_PALETTEDATA ((wxPaletteRefData *)m_refData)

class WXDLLEXPORT wxPalette: public wxPaletteBase
{
    DECLARE_DYNAMIC_CLASS(wxPalette)

public:
    wxPalette();

    wxPalette(int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue);
    virtual ~wxPalette();
    bool Create(int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue);
    int GetPixel(unsigned char red, unsigned char green, unsigned char blue) const;
    bool GetRGB(int pixel, unsigned char *red, unsigned char *green, unsigned char *blue) const;

    virtual bool Ok() const { return IsOk(); }
    virtual bool IsOk() const { return (m_refData != NULL) ; }

    // X-specific
    WXColormap GetXColormap(WXDisplay* display = NULL) const;
    bool TransferBitmap(void *data, int depth, int size);
    bool TransferBitmap8(unsigned char *data, unsigned long size, void *dest, unsigned int bpp);
    unsigned long *GetXPixArray(WXDisplay* display, int *pix_array_n);
    void PutXColormap(WXDisplay* display, WXColormap cmap, bool destroyable);
};

#endif
// _WX_PALETTE_H_
