/////////////////////////////////////////////////////////////////////////////
// Name:        wx/motif/font.h
// Purpose:     wxFont class
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// RCS-ID:      $Id: font.h 39411 2006-05-29 00:03:36Z VZ $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FONT_H_
#define _WX_FONT_H_

#if __WXMOTIF20__ && !__WXLESSTIF__
    #define wxMOTIF_NEW_FONT_HANDLING 1
    #define wxMOTIF_USE_RENDER_TABLE 1
#else
    #define wxMOTIF_NEW_FONT_HANDLING 0
    #define wxMOTIF_USE_RENDER_TABLE 0
#endif

class wxXFont;

// Font
class WXDLLIMPEXP_CORE wxFont : public wxFontBase
{
public:
    // ctors and such
    wxFont() { }

    wxFont(int size,
        int family,
        int style,
        int weight,
        bool underlined = false,
        const wxString& face = wxEmptyString,
        wxFontEncoding encoding = wxFONTENCODING_DEFAULT)
    {
        (void)Create(size, family, style, weight, underlined, face, encoding);
    }

    wxFont(const wxNativeFontInfo& info);

    bool Create(int size,
        int family,
        int style,
        int weight,
        bool underlined = false,
        const wxString& face = wxEmptyString,
        wxFontEncoding encoding = wxFONTENCODING_DEFAULT);

    // wxMOTIF-specific
    bool Create(const wxString& fontname,
        wxFontEncoding fontenc = wxFONTENCODING_DEFAULT);
    bool Create(const wxNativeFontInfo& fontinfo);

    virtual ~wxFont();

    // implement base class pure virtuals
    virtual int GetPointSize() const;
    virtual int GetFamily() const;
    virtual int GetStyle() const;
    virtual int GetWeight() const;
    virtual bool GetUnderlined() const;
    virtual wxString GetFaceName() const;
    virtual wxFontEncoding GetEncoding() const;
    virtual const wxNativeFontInfo *GetNativeFontInfo() const;

    virtual void SetPointSize(int pointSize);
    virtual void SetFamily(int family);
    virtual void SetStyle(int style);
    virtual void SetWeight(int weight);
    virtual bool SetFaceName(const wxString& faceName);
    virtual void SetUnderlined(bool underlined);
    virtual void SetEncoding(wxFontEncoding encoding);

    // Implementation

    // Find an existing, or create a new, XFontStruct
    // based on this wxFont and the given scale. Append the
    // font to list in the private data for future reference.

    // TODO This is a fairly basic implementation, that doesn't
    // allow for different facenames, and also doesn't do a mapping
    // between 'standard' facenames (e.g. Arial, Helvetica, Times Roman etc.)
    // and the fonts that are available on a particular system.
    // Maybe we need to scan the user's machine to build up a profile
    // of the fonts and a mapping file.

    // Return font struct, and optionally the Motif font list
    wxXFont *GetInternalFont(double scale = 1.0,
        WXDisplay* display = NULL) const;

    // These two are helper functions for convenient access of the above.
#if wxMOTIF_NEW_FONT_HANDLING
    WXFontSet GetFontSet(double scale, WXDisplay* display = NULL) const;
#else // if !wxMOTIF_NEW_FONT_HANDLING
    WXFontStructPtr GetFontStruct(double scale = 1.0,
        WXDisplay* display = NULL) const;
#endif // wxMOTIF_NEW_FONT_HANDLING
#if wxMOTIF_USE_RENDER_TABLE
    WXRenderTable GetRenderTable(WXDisplay* display) const;
#else // if !wxMOTIF_USE_RENDER_TABLE
    WXFontList GetFontList(double scale = 1.0,
        WXDisplay* display = NULL) const;
#endif // wxMOTIF_USE_RENDER_TABLE
    // returns either a XmFontList or XmRenderTable, depending
    // on Motif version
    WXFontType GetFontType(WXDisplay* display) const;
    // like the function above but does a copy for XmFontList
    WXFontType GetFontTypeC(WXDisplay* display) const;
    static WXString GetFontTag();
protected:
    virtual void DoSetNativeFontInfo( const wxNativeFontInfo& info );

    void Unshare();

private:
    DECLARE_DYNAMIC_CLASS(wxFont)
};

#endif
// _WX_FONT_H_
