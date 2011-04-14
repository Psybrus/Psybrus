/////////////////////////////////////////////////////////////////////////////
// Name:        src/mac/carbon/control.cpp
// Purpose:     wxControl class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: control.cpp 39627 2006-06-08 06:57:39Z ABX $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "wx/control.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/panel.h"
    #include "wx/dc.h"
    #include "wx/dcclient.h"
    #include "wx/button.h"
    #include "wx/dialog.h"
    #include "wx/scrolbar.h"
    #include "wx/stattext.h"
    #include "wx/statbox.h"
    #include "wx/radiobox.h"
    #include "wx/sizer.h"
#endif // WX_PRECOMP

#include "wx/notebook.h"
#include "wx/tabctrl.h"
#include "wx/spinbutt.h"

#include "wx/mac/uma.h"
#include "wx/mac/private.h"

IMPLEMENT_ABSTRACT_CLASS(wxControl, wxWindow)


wxControl::wxControl()
{
}

bool wxControl::Create( wxWindow *parent,
       wxWindowID id,
       const wxPoint& pos,
       const wxSize& size,
       long style,
       const wxValidator& validator,
       const wxString& name )
{
    bool rval = wxWindow::Create( parent, id, pos, size, style, name );

#if 0
    // no automatic inheritance as we most often need transparent backgrounds
    if ( parent )
    {
        m_backgroundColour = parent->GetBackgroundColour();
        m_foregroundColour = parent->GetForegroundColour();
    }
#endif

#if wxUSE_VALIDATORS
    if (rval)
        SetValidator( validator );
#endif

    return rval;
}

wxControl::~wxControl()
{
    m_isBeingDeleted = true;
}

bool wxControl::ProcessCommand( wxCommandEvent &event )
{
    // Tries:
    // 1) OnCommand, starting at this window and working up parent hierarchy
    // 2) OnCommand then calls ProcessEvent to search the event tables.
    return GetEventHandler()->ProcessEvent( event );
}

void  wxControl::OnKeyDown( wxKeyEvent &event )
{
    if ( m_peer == NULL || !m_peer->Ok() )
        return;

    UInt32 keyCode, modifiers;
    char charCode;

    GetEventParameter( (EventRef)wxTheApp->MacGetCurrentEvent(), kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &keyCode  );
    GetEventParameter( (EventRef)wxTheApp->MacGetCurrentEvent(), kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &charCode );
    GetEventParameter( (EventRef)wxTheApp->MacGetCurrentEvent(), kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifiers );

    m_peer->HandleKey( keyCode, charCode, modifiers );
}
