/////////////////////////////////////////////////////////////////////////////
// Name:        wx/mac/carbon/colour.h
// Purpose:     wxColour class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: colour.h 44007 2006-12-19 05:43:19Z SC $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COLOUR_H_
#define _WX_COLOUR_H_

#include "wx/object.h"
#include "wx/string.h"

// Colour
class WXDLLEXPORT wxColour: public wxColourBase
{
public:
    // constructors
    // ------------

    // default
    wxColour() { Init(); }
    DEFINE_STD_WXCOLOUR_CONSTRUCTORS

    // dtor
    virtual ~wxColour();

    // accessors
    bool Ok() const { return IsOk(); }
    bool IsOk() const;

    unsigned char Red() const { return m_red; }
    unsigned char Green() const { return m_green; }
    unsigned char Blue() const { return m_blue; }
    unsigned char Alpha() const { return m_alpha; }

    // comparison
    bool operator == (const wxColour& colour) const
    {
        return (m_isInit == colour.m_isInit
                && m_red == colour.m_red
                && m_green == colour.m_green
                && m_blue == colour.m_blue
                && m_alpha == colour.m_alpha);
    }
    bool operator != (const wxColour& colour) const { return !(*this == colour); }

    const WXCOLORREF& GetPixel() const { return m_pixel; };

protected :

    // Helper function
    void Init();

    virtual void
    InitRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

private:
    bool          m_isInit;
    unsigned char m_red;
    unsigned char m_blue;
    unsigned char m_green;
    unsigned char m_alpha;

public:
    WXCOLORREF m_pixel ;
    void FromRGBColor( WXCOLORREF* color ) ;


private:
    DECLARE_DYNAMIC_CLASS(wxColour)
};

#endif
  // _WX_COLOUR_H_
