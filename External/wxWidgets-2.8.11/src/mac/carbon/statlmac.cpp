/////////////////////////////////////////////////////////////////////////////
// Name:        src/mac/carbon/statlmac.cpp
// Purpose:     a generic wxStaticLine class
// Author:      Vadim Zeitlin
// Created:     28.06.99
// Version:     $Id: statlmac.cpp 39487 2006-05-31 18:27:51Z ABX $
// Copyright:   (c) 1998 Vadim Zeitlin
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

#if wxUSE_STATLINE

#include "wx/statline.h"

#ifndef WX_PRECOMP
    #include "wx/statbox.h"
#endif

#include "wx/mac/uma.h"

// ============================================================================
// implementation
// ============================================================================

IMPLEMENT_DYNAMIC_CLASS(wxStaticLine, wxControl)

// ----------------------------------------------------------------------------
// wxStaticLine
// ----------------------------------------------------------------------------

bool wxStaticLine::Create( wxWindow *parent,
                           wxWindowID id,
                           const wxPoint &pos,
                           const wxSize &size,
                           long style,
                           const wxString &name)
{
    m_macIsUserPane = false ;

    if ( !wxStaticLineBase::Create(parent, id, pos, size,
                                   style, wxDefaultValidator, name) )
        return false;

    Rect bounds = wxMacGetBoundsForControl( this , pos , size ) ;
    m_peer = new wxMacControl(this) ;
    verify_noerr(CreateSeparatorControl(MAC_WXHWND(parent->MacGetTopLevelWindowRef()),&bounds, m_peer->GetControlRefAddr() ) ) ;

    MacPostControlCreate(pos,size) ;

    return true;
}

#endif //wxUSE_STATLINE
