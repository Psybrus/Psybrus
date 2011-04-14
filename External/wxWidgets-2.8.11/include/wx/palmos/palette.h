/////////////////////////////////////////////////////////////////////////////
// Name:        wx/palmos/palette.h
// Purpose:     wxPalette class
// Author:      William Osborne - minimal working wxPalmOS port
// Modified by:
// Created:     10/13/04
// RCS-ID:      $Id: palette.h 42752 2006-10-30 19:26:48Z VZ $
// Copyright:   (c) William Osborne
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PALETTE_H_
#define _WX_PALETTE_H_

#include "wx/gdiobj.h"

class WXDLLEXPORT wxPalette;

class WXDLLEXPORT wxPaletteRefData: public wxGDIRefData
{
    friend class WXDLLEXPORT wxPalette;
public:
    wxPaletteRefData(void);
    virtual ~wxPaletteRefData(void);
protected:
 WXHPALETTE m_hPalette;
};

#define M_PALETTEDATA ((wxPaletteRefData *)m_refData)

class WXDLLEXPORT wxPalette: public wxPaletteBase
{
  DECLARE_DYNAMIC_CLASS(wxPalette)

public:
  wxPalette(void);

  wxPalette(int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue);
  virtual ~wxPalette(void);
  bool Create(int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue);
  int GetPixel(unsigned char red, unsigned char green, unsigned char blue) const;
  bool GetRGB(int pixel, unsigned char *red, unsigned char *green, unsigned char *blue) const;

  virtual bool Ok() const { return IsOk(); }
  virtual bool IsOk(void) const { return (m_refData != NULL) ; }

  virtual bool FreeResource(bool force = false);

  inline WXHPALETTE GetHPALETTE(void) const { return (M_PALETTEDATA ? M_PALETTEDATA->m_hPalette : 0); }
  void SetHPALETTE(WXHPALETTE pal);
};

#endif
    // _WX_PALETTE_H_
