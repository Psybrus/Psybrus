/////////////////////////////////////////////////////////////////////////////
// Name:        src/mac/carbon/stattext.cpp
// Purpose:     wxStaticText
// Author:      Stefan Csomor
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: stattext.cpp 57849 2009-01-06 09:36:54Z SC $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#if wxUSE_STATTEXT

#include "wx/stattext.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/utils.h"
    #include "wx/dc.h"
    #include "wx/dcclient.h"
    #include "wx/settings.h"
#endif // WX_PRECOMP

#include "wx/notebook.h"
#include "wx/tabctrl.h"

#include "wx/mac/uma.h"

#include <stdio.h>

IMPLEMENT_DYNAMIC_CLASS(wxStaticText, wxControl)


bool wxStaticText::Create( wxWindow *parent,
    wxWindowID id,
    const wxString& label,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name )
{
    m_macIsUserPane = false;

    m_label = GetLabelText( label );

    if ( !wxControl::Create( parent, id, pos, size, style, wxDefaultValidator, name ) )
        return false;

    Rect bounds = wxMacGetBoundsForControl( this, pos, size );
    wxMacCFStringHolder str( m_label, GetFont().GetEncoding() );

    m_peer = new wxMacControl( this );
    OSStatus err = CreateStaticTextControl(
        MAC_WXHWND(parent->MacGetTopLevelWindowRef()),
        &bounds, str, NULL, m_peer->GetControlRefAddr() );
    verify_noerr( err );

    if ( ( style & wxST_DOTS_END ) || ( style & wxST_DOTS_MIDDLE ) )
    {
        TruncCode tCode = truncEnd;
        if ( style & wxST_DOTS_MIDDLE )
            tCode = truncMiddle;

        err = m_peer->SetData( kControlStaticTextTruncTag, tCode );
        err = m_peer->SetData( kControlStaticTextIsMultilineTag, (Boolean)0 );
    }

    MacPostControlCreate( pos, size );

    return true;
}

wxSize wxStaticText::DoGetBestSize() const
{
    Rect bestsize = { 0 , 0 , 0 , 0 } ;
    Point bounds;
    
    // try the built-in best size if available
    Boolean former = m_peer->GetData<Boolean>( kControlStaticTextIsMultilineTag);
    m_peer->SetData( kControlStaticTextIsMultilineTag, (Boolean)0 );
    m_peer->GetBestRect( &bestsize ) ;
    m_peer->SetData( kControlStaticTextIsMultilineTag, former );
    if ( !EmptyRect( &bestsize ) )
    {
        bounds.h = bestsize.right - bestsize.left ;
        bounds.v = bestsize.bottom - bestsize.top ;
    }
    else
    {
        ControlFontStyleRec controlFont;
        OSStatus err = m_peer->GetData<ControlFontStyleRec>( kControlEntireControl, kControlFontStyleTag, &controlFont );
        verify_noerr( err );

        SInt16 baseline;
        wxMacCFStringHolder str( m_label,  GetFont().GetEncoding() );

#ifndef __LP64__
        if ( m_font.MacGetThemeFontID() != kThemeCurrentPortFont )
        {
            err = GetThemeTextDimensions(
                (!m_label.empty() ? (CFStringRef)str : CFSTR(" ")),
                m_font.MacGetThemeFontID(), kThemeStateActive, false, &bounds, &baseline );
            verify_noerr( err );
        }
        else
#endif
        {
    #if wxMAC_USE_CORE_GRAPHICS
            wxClientDC dc(const_cast<wxStaticText*>(this));
            wxCoord width, height ;
            dc.GetTextExtent( m_label , &width, &height);
            bounds.h = width;
            bounds.v = height;
    #else
            wxMacWindowStateSaver sv( this );
            ::TextFont( m_font.MacGetFontNum() );
            ::TextSize( (short)(m_font.MacGetFontSize()) );
            ::TextFace( m_font.MacGetFontStyle() );

            err = GetThemeTextDimensions(
                (!m_label.empty() ? (CFStringRef)str : CFSTR(" ")),
                kThemeCurrentPortFont, kThemeStateActive, false, &bounds, &baseline );
            verify_noerr( err );
    #endif
        }

        if ( m_label.empty() )
            bounds.h = 0;
    }
    bounds.h += MacGetLeftBorderSize() + MacGetRightBorderSize();
    bounds.v += MacGetTopBorderSize() + MacGetBottomBorderSize();

    return wxSize( bounds.h, bounds.v );
}

void wxStaticText::SetLabel( const wxString& st )
{
    m_label =  st;

    wxMacCFStringHolder str( GetLabelText(m_label), GetFont().GetEncoding() );
    CFStringRef ref = str;
    OSStatus err = m_peer->SetData<CFStringRef>(kControlEntireControl, kControlStaticTextCFStringTag, ref );
    verify_noerr( err );

    if ( !(GetWindowStyle() & wxST_NO_AUTORESIZE) )
    {
        InvalidateBestSize();
        SetSize( GetBestSize() );
    }

    Refresh();

    // we shouldn't need forced updates
    // Update();
}

bool wxStaticText::SetFont(const wxFont& font)
{
    bool ret = wxControl::SetFont( font );

    if ( ret )
    {
        if ( !(GetWindowStyle() & wxST_NO_AUTORESIZE) )
        {
            InvalidateBestSize();
            SetSize( GetBestSize() );
        }
    }

    return ret;
}

#endif //if wxUSE_STATTEXT
