/////////////////////////////////////////////////////////////////////////////
// Name:        font.h
// Purpose:     wxFont class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// RCS-ID:      $Id: font.h 39411 2006-05-29 00:03:36Z VZ $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FONT_H_
#define _WX_FONT_H_

// ----------------------------------------------------------------------------
// wxFont
// ----------------------------------------------------------------------------

class WXDLLEXPORT wxFont : public wxFontBase
{
public:
    // ctors and such
    wxFont() { }

    wxFont(int size,
           int family,
           int style,
           int weight,
           bool underlined = FALSE,
           const wxString& face = wxEmptyString,
           wxFontEncoding encoding = wxFONTENCODING_DEFAULT)
    {
        (void)Create(size, family, style, weight, underlined, face, encoding);
    }

    wxFont(const wxNativeFontInfo& info)
    {
        (void)Create(info);
    }

    wxFont(const wxString& fontDesc);

    bool Create(int size,
                int family,
                int style,
                int weight,
                bool underlined = FALSE,
                const wxString& face = wxEmptyString,
                wxFontEncoding encoding = wxFONTENCODING_DEFAULT);

    bool Create(const wxNativeFontInfo& info);

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

    // implementation only from now on
    // -------------------------------

    virtual bool RealizeResource();

    // Unofficial API, don't use
    virtual void SetNoAntiAliasing( bool noAA = TRUE ) ;
    virtual bool GetNoAntiAliasing() const ;

    // Mac-specific, risks to change, don't use in portable code
    short GetMacFontNum() const;
    short GetMacFontSize() const;
    wxByte  GetMacFontStyle() const;
    wxUint32 GetMacATSUFontID() const;

protected:
    void Unshare();

private:
    DECLARE_DYNAMIC_CLASS(wxFont)
};

#endif
    // _WX_FONT_H_
