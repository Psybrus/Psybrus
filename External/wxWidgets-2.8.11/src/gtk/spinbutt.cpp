/////////////////////////////////////////////////////////////////////////////
// Name:        src/gtk/spinbutt.cpp
// Purpose:     wxSpinButton
// Author:      Robert
// Modified by:
// RCS-ID:      $Id: spinbutt.cpp 48667 2007-09-14 04:11:01Z PC $
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_SPINBTN

#include "wx/spinbutt.h"

#ifndef WX_PRECOMP
    #include "wx/utils.h"
#endif

#include "wx/gtk/private.h"

//-----------------------------------------------------------------------------
// data
//-----------------------------------------------------------------------------

extern bool   g_blockEventsOnDrag;

//-----------------------------------------------------------------------------
// "value_changed"
//-----------------------------------------------------------------------------

extern "C" {
static void
gtk_value_changed(GtkSpinButton* spinbutton, wxSpinButton* win)
{
    if (g_isIdle) wxapp_install_idle_handler();

    const double value = gtk_spin_button_get_value(spinbutton);
    const int pos = int(value);
    const int oldPos = win->m_pos;
    if (!win->m_hasVMT || g_blockEventsOnDrag || win->m_blockScrollEvent || pos == oldPos)
    {
        win->m_pos = pos;
        return;
    }

    wxSpinEvent event(pos > oldPos ? wxEVT_SCROLL_LINEUP : wxEVT_SCROLL_LINEDOWN, win->GetId());
    event.SetPosition(pos);
    event.SetEventObject(win);

    if ((win->GetEventHandler()->ProcessEvent( event )) &&
        !event.IsAllowed() )
    {
        /* program has vetoed */
        win->BlockScrollEvent();
        gtk_spin_button_set_value(spinbutton, oldPos);
        win->UnblockScrollEvent();
        return;
    }

    win->m_pos = pos;

    /* always send a thumbtrack event */
    wxSpinEvent event2(wxEVT_SCROLL_THUMBTRACK, win->GetId());
    event2.SetPosition(pos);
    event2.SetEventObject(win);
    win->GetEventHandler()->ProcessEvent(event2);
}
}

//-----------------------------------------------------------------------------
// wxSpinButton
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxSpinButton,wxControl)
IMPLEMENT_DYNAMIC_CLASS(wxSpinEvent, wxNotifyEvent)

BEGIN_EVENT_TABLE(wxSpinButton, wxControl)
    EVT_SIZE(wxSpinButton::OnSize)
END_EVENT_TABLE()

wxSpinButton::wxSpinButton()
{
    m_pos = 0;
}

bool wxSpinButton::Create(wxWindow *parent,
                          wxWindowID id,
                          const wxPoint& pos,
                          const wxSize& size,
                          long style,
                          const wxString& name)
{
    m_needParent = true;

    wxSize new_size = size,
           sizeBest = DoGetBestSize();
    new_size.x = sizeBest.x;            // override width always
    if (new_size.y == -1)
        new_size.y = sizeBest.y;

    if (!PreCreation( parent, pos, new_size ) ||
        !CreateBase( parent, id, pos, new_size, style, wxDefaultValidator, name ))
    {
        wxFAIL_MSG( wxT("wxSpinButton creation failed") );
        return false;
    }

    m_pos = 0;

    m_widget = gtk_spin_button_new_with_range(0, 100, 1);

    gtk_spin_button_set_wrap( GTK_SPIN_BUTTON(m_widget),
                              (int)(m_windowStyle & wxSP_WRAP) );

    g_signal_connect_after(
        m_widget, "value_changed", G_CALLBACK(gtk_value_changed), this);

    m_parent->DoAddChild( this );

    PostCreation(new_size);

    return true;
}

int wxSpinButton::GetMin() const
{
    wxCHECK_MSG( (m_widget != NULL), 0, wxT("invalid spin button") );

    double min;
    gtk_spin_button_get_range((GtkSpinButton*)m_widget, &min, NULL);
    return int(min);
}

int wxSpinButton::GetMax() const
{
    wxCHECK_MSG( (m_widget != NULL), 0, wxT("invalid spin button") );

    double max;
    gtk_spin_button_get_range((GtkSpinButton*)m_widget, NULL, &max);
    return int(max);
}

int wxSpinButton::GetValue() const
{
    wxCHECK_MSG( (m_widget != NULL), 0, wxT("invalid spin button") );

    return m_pos;
}

static void wxSpinButton_GtkDisableEvents( const wxSpinButton *spinctrl )
{
    g_signal_handlers_block_by_func( spinctrl->m_widget,
        (gpointer)gtk_value_changed, (void*) spinctrl );
}

static void wxSpinButton_GtkEnableEvents( const wxSpinButton *spinctrl )
{
    g_signal_handlers_unblock_by_func( spinctrl->m_widget,
        (gpointer)gtk_value_changed, (void*) spinctrl );
}

void wxSpinButton::SetValue( int value )
{
    wxCHECK_RET( (m_widget != NULL), wxT("invalid spin button") );

    wxSpinButton_GtkDisableEvents( this );
    gtk_spin_button_set_value((GtkSpinButton*)m_widget, value);
    m_pos = int(gtk_spin_button_get_value((GtkSpinButton*)m_widget));
    wxSpinButton_GtkEnableEvents( this );
}

void wxSpinButton::SetRange(int minVal, int maxVal)
{
    wxCHECK_RET( (m_widget != NULL), wxT("invalid spin button") );

    wxSpinButton_GtkDisableEvents( this );
    gtk_spin_button_set_range((GtkSpinButton*)m_widget, minVal, maxVal);
    m_pos = int(gtk_spin_button_get_value((GtkSpinButton*)m_widget));
    wxSpinButton_GtkEnableEvents( this );
}

void wxSpinButton::OnSize( wxSizeEvent &WXUNUSED(event) )
{
    wxCHECK_RET( (m_widget != NULL), wxT("invalid spin button") );

    m_width = DoGetBestSize().x;
    gtk_widget_set_size_request( m_widget, m_width, m_height );
}

GdkWindow *wxSpinButton::GTKGetWindow(wxArrayGdkWindows& WXUNUSED(windows)) const
{
    return GTK_SPIN_BUTTON(m_widget)->panel;
}

wxSize wxSpinButton::DoGetBestSize() const
{
    wxSize best(15, 26); // FIXME
    CacheBestSize(best);
    return best;
}

// static
wxVisualAttributes
wxSpinButton::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    // TODO: overload to accept functions like gtk_spin_button_new?
    // Until then use a similar type
    return GetDefaultAttributesFromGTKWidget(gtk_button_new);
}

#endif
