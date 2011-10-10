/////////////////////////////////////////////////////////////////////////////
// Name:        src/gtk/checkbox.cpp
// Purpose:
// Author:      Robert Roebling
// Id:          $Id: checkbox.cpp 58188 2009-01-17 20:38:43Z JS $
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_CHECKBOX

#include "wx/checkbox.h"

#include "wx/gtk/private.h"

//-----------------------------------------------------------------------------
// data
//-----------------------------------------------------------------------------

extern bool           g_blockEventsOnDrag;

//-----------------------------------------------------------------------------
// "clicked"
//-----------------------------------------------------------------------------

extern "C" {
static void gtk_checkbox_toggled_callback(GtkWidget *widget, wxCheckBox *cb)
{
    if (g_isIdle) wxapp_install_idle_handler();

    if (!cb->m_hasVMT) return;

    if (g_blockEventsOnDrag) return;

    if (cb->m_blockEvent) return;

    // Transitions for 3state checkbox must be done manually, GTK's checkbox
    // is 2state with additional "undetermined state" flag which isn't
    // changed automatically:
    if (cb->Is3State())
    {
        GtkToggleButton *toggle = GTK_TOGGLE_BUTTON(widget);

        if (cb->Is3rdStateAllowedForUser())
        {
            // The 3 states cycle like this when clicked:
            // checked -> undetermined -> unchecked -> checked -> ...
            bool active = gtk_toggle_button_get_active(toggle);
            bool inconsistent = gtk_toggle_button_get_inconsistent(toggle);

            cb->m_blockEvent = true;

            if (!active && !inconsistent)
            {
                // checked -> undetermined
                gtk_toggle_button_set_active(toggle, true);
                gtk_toggle_button_set_inconsistent(toggle, true);
            }
            else if (!active && inconsistent)
            {
                // undetermined -> unchecked
                gtk_toggle_button_set_inconsistent(toggle, false);
            }
            else if (active && !inconsistent)
            {
                // unchecked -> checked
                // nothing to do
            }
            else
            {
                wxFAIL_MSG(_T("3state wxCheckBox in unexpected state!"));
            }

            cb->m_blockEvent = false;
        }
        else
        {
            // user's action unsets undetermined state:
            gtk_toggle_button_set_inconsistent(toggle, false);
        }
    }

    wxCommandEvent event(wxEVT_COMMAND_CHECKBOX_CLICKED, cb->GetId());
    event.SetInt(cb->Get3StateValue());
    event.SetEventObject(cb);
    cb->GetEventHandler()->ProcessEvent(event);
}
}

//-----------------------------------------------------------------------------
// wxCheckBox
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxCheckBox,wxControl)

wxCheckBox::wxCheckBox()
{
}

bool wxCheckBox::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxString &label,
                        const wxPoint &pos,
                        const wxSize &size,
                        long style,
                        const wxValidator& validator,
                        const wxString &name )
{
    m_needParent = true;
    m_acceptsFocus = true;
    m_blockEvent = false;

    if (!PreCreation( parent, pos, size ) ||
        !CreateBase( parent, id, pos, size, style, validator, name ))
    {
        wxFAIL_MSG( wxT("wxCheckBox creation failed") );
        return false;
    }

    wxASSERT_MSG( (style & wxCHK_ALLOW_3RD_STATE_FOR_USER) == 0 ||
                  (style & wxCHK_3STATE) != 0,
                  wxT("Using wxCHK_ALLOW_3RD_STATE_FOR_USER")
                  wxT(" style flag for a 2-state checkbox is useless") );

    if ( style & wxALIGN_RIGHT )
    {
        // VZ: as I don't know a way to create a right aligned checkbox with
        //     GTK we will create a checkbox without label and a label at the
        //     left of it
        m_widgetCheckbox = gtk_check_button_new();

        m_widgetLabel = gtk_label_new("");
        gtk_misc_set_alignment(GTK_MISC(m_widgetLabel), 0.0, 0.5);

        m_widget = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(m_widget), m_widgetLabel, FALSE, FALSE, 3);
        gtk_box_pack_start(GTK_BOX(m_widget), m_widgetCheckbox, FALSE, FALSE, 3);

        gtk_widget_show( m_widgetLabel );
        gtk_widget_show( m_widgetCheckbox );
    }
    else
    {
        m_widgetCheckbox = gtk_check_button_new_with_label("");
        m_widgetLabel = GTK_BIN(m_widgetCheckbox)->child;
        m_widget = m_widgetCheckbox;
    }
    SetLabel( label );

    g_signal_connect (m_widgetCheckbox, "toggled",
                      G_CALLBACK (gtk_checkbox_toggled_callback), this);

    m_parent->DoAddChild( this );

    PostCreation(size);

    return true;
}

void wxCheckBox::SetValue( bool state )
{
    wxCHECK_RET( m_widgetCheckbox != NULL, wxT("invalid checkbox") );

    if (state == GetValue())
        return;

    m_blockEvent = true;

    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(m_widgetCheckbox), state );

    m_blockEvent = false;
}

bool wxCheckBox::GetValue() const
{
    wxCHECK_MSG( m_widgetCheckbox != NULL, false, wxT("invalid checkbox") );

    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_widgetCheckbox));
}

void wxCheckBox::DoSet3StateValue(wxCheckBoxState state)
{
    SetValue(state != wxCHK_UNCHECKED);
    gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(m_widgetCheckbox),
                                       state == wxCHK_UNDETERMINED);
}

wxCheckBoxState wxCheckBox::DoGet3StateValue() const
{
    if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(m_widgetCheckbox)))
    {
        return wxCHK_UNDETERMINED;
    }
    else
    {
        return GetValue() ? wxCHK_CHECKED : wxCHK_UNCHECKED;
    }
}

void wxCheckBox::SetLabel( const wxString& label )
{
    wxCHECK_RET( m_widgetLabel != NULL, wxT("invalid checkbox") );

    GTKSetLabelForLabel(GTK_LABEL(m_widgetLabel), label);
}

bool wxCheckBox::Enable( bool enable )
{
    bool isEnabled = IsEnabled();

    if ( !wxControl::Enable( enable ) )
        return false;

    gtk_widget_set_sensitive( m_widgetLabel, enable );

    if (!isEnabled && enable)
        wxGtkFixSensitivity(this);

    return true;
}

void wxCheckBox::DoApplyWidgetStyle(GtkRcStyle *style)
{
    gtk_widget_modify_style(m_widgetCheckbox, style);
    gtk_widget_modify_style(m_widgetLabel, style);
}

GdkWindow *wxCheckBox::GTKGetWindow(wxArrayGdkWindows& WXUNUSED(windows)) const
{
    return GTK_BUTTON(m_widgetCheckbox)->event_window;
}

wxSize wxCheckBox::DoGetBestSize() const
{
    return wxControl::DoGetBestSize();
}

// static
wxVisualAttributes
wxCheckBox::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    return GetDefaultAttributesFromGTKWidget(gtk_check_button_new);
}

#endif
