/////////////////////////////////////////////////////////////////////////////
// Name:        wx/cocoa/colour.h
// Purpose:     wxColour class
// Author:      David Elliott
// Modified by:
// Created:     2003/06/17
// RCS-ID:      $Id: colour.h 41848 2006-10-09 23:44:00Z VZ $
// Copyright:   (c) 2003 David Elliott
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_COCOA_COLOUR_H__
#define __WX_COCOA_COLOUR_H__

#include "wx/object.h"
#include "wx/string.h"

// ========================================================================
// wxColour
// ========================================================================

class WXDLLEXPORT wxColour : public wxColourBase
{
public:
    // constructors
    // ------------

    // default
    wxColour() { Init(); }

    // the other standard ones: notice that we can't use
    // DEFINE_STD_WXCOLOUR_CONSTRUCTORS here because we need to call Init() to
    // initialize m_cocoaNSColor and the macro doesn't do it
    wxColour( ChannelType red, ChannelType green, ChannelType blue,
              ChannelType alpha = wxALPHA_OPAQUE )
        { Init(); Set(red, green, blue, alpha); }
    wxColour(unsigned long colRGB) { Init(); Set(colRGB); }
    wxColour(const wxString &colourName) { Init(); Set(colourName); }
    wxColour(const wxChar *colourName) { Init(); Set(colourName); }

    // initialization using existing NSColor
    wxColour( WX_NSColor aColor );


    // copy ctors and assignment operators
    wxColour( const wxColour& col );
    wxColour& operator = ( const wxColour& col );

    virtual ~wxColour();

    // accessors
    bool Ok() const { return IsOk(); }
    bool IsOk() const { return m_cocoaNSColor; }
    WX_NSColor GetNSColor() { return m_cocoaNSColor; }

    unsigned char Red() const { return m_red; }
    unsigned char Green() const { return m_green; }
    unsigned char Blue() const { return m_blue; }
    unsigned char Alpha() const { return m_alpha; }

    // comparison
    bool operator == (const wxColour& colour) const
    {
        return m_cocoaNSColor == colour.m_cocoaNSColor ||
               (m_red == colour.m_red &&
                m_green == colour.m_green &&
                m_blue == colour.m_blue &&
                m_alpha == colour.m_alpha);
    }
    bool operator != (const wxColour& colour) const
        { return !(*this == colour); }

    // Set() functions
    void Set( WX_NSColor aColor );

    // reroute the inherited ones
    void Set(unsigned char red,
             unsigned char green,
             unsigned char blue,
             unsigned char alpha = wxALPHA_OPAQUE)
        { wxColourBase::Set(red, green, blue, alpha); }

    bool Set(const wxChar *str)
        { return wxColourBase::Set(str); }

    bool Set(const wxString &str)
        { return wxColourBase::Set(str); }

    void Set(unsigned long colRGB)
        { wxColourBase::Set(colRGB); }

protected:
    // puts the object in an invalid, uninitialized state
    void Init();

    virtual void
    InitRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

private:
    WX_NSColor m_cocoaNSColor;
    unsigned char m_red;
    unsigned char m_green;
    unsigned char m_blue;
    unsigned char m_alpha;

    DECLARE_DYNAMIC_CLASS(wxColour)
};

#endif // __WX_COCOA_COLOUR_H__
