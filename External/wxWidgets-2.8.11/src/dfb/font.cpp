/////////////////////////////////////////////////////////////////////////////
// Name:        src/dfb/font.cpp
// Purpose:     wxFont implementation
// Author:      Vaclav Slavik
// Created:     2006-08-08
// RCS-ID:      $Id: font.cpp 43547 2006-11-20 16:43:55Z VS $
// Copyright:   (c) 2006 REA Elektronik GmbH
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/font.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif

#include "wx/dfb/private.h"
#include "wx/private/fontmgr.h"

typedef wxFontMgrFontRefData wxFontRefData;
#define M_FONTDATA ((wxFontRefData*)m_refData)

// ----------------------------------------------------------------------------
// wxFont
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxFont, wxGDIObject)

bool wxFont::Create(const wxNativeFontInfo& info)
{
    return Create(info.pointSize, info.family, info.style, info.weight,
                  info.underlined, info.faceName, info.encoding);
}

bool wxFont::Create(int pointSize,
                    int family,
                    int style,
                    int weight,
                    bool underlined,
                    const wxString& face,
                    wxFontEncoding encoding)
{
    m_refData = new wxFontRefData(pointSize, family, style, weight,
                                  underlined, face, encoding);
    return true;
}

wxObjectRefData *wxFont::CreateRefData() const
{
    return new wxFontRefData;
}

wxObjectRefData *wxFont::CloneRefData(const wxObjectRefData *data) const
{
    return new wxFontRefData(*(wxFontRefData *)data);
}


// ----------------------------------------------------------------------------
// accessors
// ----------------------------------------------------------------------------

wxIDirectFBFontPtr wxFont::GetDirectFBFont(bool antialiased) const
{
    wxCHECK_MSG( Ok(), NULL, wxT("invalid font") );

    // we don't support DC scaling yet, so use scale=1
    wxFontInstance *i = M_FONTDATA->GetFontInstance(1.0, antialiased);
    return i ? i->GetDirectFBFont() : wxIDirectFBFontPtr();
}

int wxFont::GetPointSize() const
{
    wxCHECK_MSG( Ok(), 0, wxT("invalid font") );

    return M_FONTDATA->GetPointSize();
}

wxString wxFont::GetFaceName() const
{
    wxCHECK_MSG( Ok(), wxEmptyString, wxT("invalid font") );

    return M_FONTDATA->GetFaceName();
}

int wxFont::GetFamily() const
{
    wxCHECK_MSG( Ok(), 0, wxT("invalid font") );

    return M_FONTDATA->GetFamily();
}

int wxFont::GetStyle() const
{
    wxCHECK_MSG( Ok(), 0, wxT("invalid font") );

    return M_FONTDATA->GetStyle();
}

int wxFont::GetWeight() const
{
    wxCHECK_MSG( Ok(), 0, wxT("invalid font") );

    return M_FONTDATA->GetWeight();
}

bool wxFont::GetUnderlined() const
{
    wxCHECK_MSG( Ok(), false, wxT("invalid font") );

    return M_FONTDATA->GetUnderlined();
}


wxFontEncoding wxFont::GetEncoding() const
{
    wxCHECK_MSG( Ok(), wxFONTENCODING_DEFAULT, wxT("invalid font") );

    return M_FONTDATA->GetEncoding();
}

bool wxFont::IsFixedWidth() const
{
    wxCHECK_MSG( Ok(), false, wxT("invalid font") );

    return M_FONTDATA->IsFixedWidth();
}

const wxNativeFontInfo *wxFont::GetNativeFontInfo() const
{
    wxCHECK_MSG( Ok(), NULL, wxT("invalid font") );

    return M_FONTDATA->GetNativeFontInfo();
}

bool wxFont::GetNoAntiAliasing() const
{
    wxCHECK_MSG( Ok(), false, wxT("invalid font") );

    return M_FONTDATA->GetNoAntiAliasing();
}

// ----------------------------------------------------------------------------
// change font attributes
// ----------------------------------------------------------------------------

void wxFont::SetPointSize(int pointSize)
{
    AllocExclusive();
    M_FONTDATA->SetPointSize(pointSize);
}

void wxFont::SetFamily(int family)
{
    AllocExclusive();
    M_FONTDATA->SetFamily(family);
}

void wxFont::SetStyle(int style)
{
    AllocExclusive();
    M_FONTDATA->SetStyle(style);
}

void wxFont::SetWeight(int weight)
{
    AllocExclusive();
    M_FONTDATA->SetWeight(weight);
}

bool wxFont::SetFaceName(const wxString& faceName)
{
    AllocExclusive();
    M_FONTDATA->SetFaceName(faceName);
    return wxFontBase::SetFaceName(faceName);
}

void wxFont::SetUnderlined(bool underlined)
{
    AllocExclusive();
    M_FONTDATA->SetUnderlined(underlined);
}

void wxFont::SetEncoding(wxFontEncoding encoding)
{
    AllocExclusive();
    M_FONTDATA->SetEncoding(encoding);
}

void wxFont::SetNoAntiAliasing(bool no)
{
    AllocExclusive();
    M_FONTDATA->SetNoAntiAliasing(no);
}
