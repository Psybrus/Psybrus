/////////////////////////////////////////////////////////////////////////////
// Name:        wx/mgl/palette.h
// Purpose:
// Author:      Vaclav Slavik
// Created:     2001/03/11
// Id:          $Id: palette.h 42752 2006-10-30 19:26:48Z VZ $
// Copyright:   (c) 2001-2002 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_PALETTE_H__
#define __WX_PALETTE_H__

#include "wx/defs.h"
#include "wx/object.h"
#include "wx/gdiobj.h"
#include "wx/gdicmn.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class WXDLLEXPORT wxPalette;
struct palette_t;

//-----------------------------------------------------------------------------
// wxPalette
//-----------------------------------------------------------------------------

class WXDLLEXPORT wxPalette: public wxPaletteBase
{
    DECLARE_DYNAMIC_CLASS(wxPalette)

public:
    wxPalette();
    wxPalette(int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue);
    virtual ~wxPalette();
    virtual bool Ok() const { return IsOk(); }
    virtual bool IsOk() const;

    bool Create(int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue);
    int GetPixel(unsigned char red, unsigned char green, unsigned char blue) const;
    bool GetRGB(int pixel, unsigned char *red, unsigned char *green, unsigned char *blue) const;

    // implementation
    virtual int GetColoursCount() const;

    palette_t *GetMGLpalette_t() const;
};

#endif // __WX_PALETTE_H__
