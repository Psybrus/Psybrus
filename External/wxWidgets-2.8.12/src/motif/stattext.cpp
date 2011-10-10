/////////////////////////////////////////////////////////////////////////////
// Name:        src/motif/stattext.cpp
// Purpose:     wxStaticText
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: stattext.cpp 50982 2008-01-01 20:38:33Z VZ $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_STATTEXT

#include "wx/stattext.h"

#ifdef __VMS__
#pragma message disable nosimpint
#endif
#include <Xm/Label.h>
#ifdef __VMS__
#pragma message enable nosimpint
#endif

#include "wx/motif/private.h"

IMPLEMENT_DYNAMIC_CLASS(wxStaticText, wxControl)

bool wxStaticText::Create(wxWindow *parent, wxWindowID id,
           const wxString& label,
           const wxPoint& pos,
           const wxSize& size,
           long style,
           const wxString& name)
{
    if( !CreateControl( parent, id, pos, size, style,
                        wxDefaultValidator, name ) )
        return false;

    Widget parentWidget = (Widget) parent->GetClientWidget();

    Widget borderWidget =
        (Widget) wxCreateBorderWidget( (WXWidget)parentWidget, style );
    wxXmString text( GetLabelText( label ) );

    m_labelWidget =
        XtVaCreateManagedWidget (wxConstCast(name.c_str(), char),
            xmLabelWidgetClass,
            borderWidget ? borderWidget : parentWidget,
            wxFont::GetFontTag(), m_font.GetFontTypeC(XtDisplay(parentWidget)),
            XmNlabelString, text(),
            XmNalignment, ((style & wxALIGN_RIGHT)  ? XmALIGNMENT_END :
                          ((style & wxALIGN_CENTRE) ? XmALIGNMENT_CENTER :
                                                      XmALIGNMENT_BEGINNING)),
            XmNrecomputeSize, ((style & wxST_NO_AUTORESIZE) ? TRUE : FALSE),
            NULL);

    m_mainWidget = borderWidget ? borderWidget : m_labelWidget;

    AttachWidget (parent, m_mainWidget, (WXWidget) NULL,
                  pos.x, pos.y, size.x, size.y);

    ChangeBackgroundColour ();

    return true;
}

void wxStaticText::SetLabel(const wxString& label)
{
    wxXmString label_str(GetLabelText(label));

    // This variable means we don't need so many casts later.
    Widget widget = (Widget) m_labelWidget;

        XtVaSetValues(widget,
            XmNlabelString, label_str(),
            XmNlabelType, XmSTRING,
            NULL);
}

#endif // wxUSE_STATTEXT
