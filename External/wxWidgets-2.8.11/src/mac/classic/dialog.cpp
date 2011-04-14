/////////////////////////////////////////////////////////////////////////////
// Name:        src/mac/classic/dialog.cpp
// Purpose:     wxDialog class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: dialog.cpp 39348 2006-05-26 16:12:31Z PC $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "wx/dialog.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/utils.h"
    #include "wx/frame.h"
    #include "wx/settings.h"
#endif // WX_PRECOMP

#include "wx/mac/uma.h"

// Lists to keep track of windows, so we can disable/enable them
// for modal dialogs
wxList wxModalDialogs;

IMPLEMENT_DYNAMIC_CLASS(wxDialog, wxTopLevelWindow)

BEGIN_EVENT_TABLE(wxDialog, wxDialogBase)
  EVT_BUTTON(wxID_OK, wxDialog::OnOK)
  EVT_BUTTON(wxID_APPLY, wxDialog::OnApply)
  EVT_BUTTON(wxID_CANCEL, wxDialog::OnCancel)

  EVT_CHAR_HOOK(wxDialog::OnCharHook)

  EVT_SYS_COLOUR_CHANGED(wxDialog::OnSysColourChanged)

  EVT_CLOSE(wxDialog::OnCloseWindow)
END_EVENT_TABLE()

void wxDialog::Init()
{
    m_isModalStyle = false;
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
}

bool wxDialog::Create(wxWindow *parent, wxWindowID id,
           const wxString& title,
           const wxPoint& pos,
           const wxSize& size,
           long style,
           const wxString& name)
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));


    if ( !wxTopLevelWindow::Create(parent, id, title, pos, size, style, name) )
        return false;

    MacCreateRealWindow( title , pos , size , MacRemoveBordersFromStyle(style) & ~(wxYES|wxOK|wxNO|wxCANCEL) , name ) ;

    m_macWindowBackgroundTheme = kThemeBrushDialogBackgroundActive ;
    SetThemeWindowBackground( (WindowRef) m_macWindow , m_macWindowBackgroundTheme , false ) ;

    return true;
}

void wxDialog::SetModal(bool flag)
{
    if ( flag )
    {
        m_isModalStyle = true;

        wxModelessWindows.DeleteObject(this);
#if TARGET_CARBON
        SetWindowModality( (WindowRef) MacGetWindowRef() , kWindowModalityAppModal , NULL ) ;
#endif
    }
    else
    {
        m_isModalStyle = false;

        wxModelessWindows.Append(this);
    }
}

wxDialog::~wxDialog()
{
    m_isBeingDeleted = true;
    Show(false);
}

// By default, pressing escape cancels the dialog , on mac command-stop does the same thing
void wxDialog::OnCharHook(wxKeyEvent& event)
{
    if (( event.m_keyCode == WXK_ESCAPE ||
        ( event.m_keyCode == '.' && event.MetaDown() ) )
        && FindWindow(wxID_CANCEL) )
    {
        // Behaviour changed in 2.0: we'll send a Cancel message
        // to the dialog instead of Close.
        wxCommandEvent cancelEvent(wxEVT_COMMAND_BUTTON_CLICKED, wxID_CANCEL);
        cancelEvent.SetEventObject( this );
        GetEventHandler()->ProcessEvent(cancelEvent);

        return;
    }
    // We didn't process this event.
    event.Skip();
}

bool wxDialog::IsModal() const
{
    return m_isModalStyle;
}


bool wxDialog::IsModalShowing() const
{
    return wxModalDialogs.Find((wxDialog *)this) != NULL; // const_cast
}

bool wxDialog::Show(bool show)
{
    if ( !wxDialogBase::Show(show) )
    {
        // nothing to do
        return false;
    }

    if ( show )
    {
        // usually will result in TransferDataToWindow() being called
        InitDialog();
    }

    if ( IsModal() )
    {
        if ( show )
        {
            DoShowModal();
        }
        else // end of modal dialog
        {
            // this will cause IsModalShowing() return false and our local
            // message loop will terminate
            wxModalDialogs.DeleteObject(this);
        }
    }

    return true;
}

#if !TARGET_CARBON
extern bool s_macIsInModalLoop ;
#endif

void wxDialog::DoShowModal()
{
    wxCHECK_RET( !IsModalShowing(), _T("DoShowModal() called twice") );

    wxModalDialogs.Append(this);

#if TARGET_CARBON
    BeginAppModalStateForWindow(  (WindowRef) MacGetWindowRef()) ;
#else
    // TODO : test whether parent gets disabled
    bool formerModal = s_macIsInModalLoop ;
    s_macIsInModalLoop = true ;
#endif
    while ( IsModalShowing() )
    {
        wxTheApp->MacDoOneEvent() ;
        // calls process idle itself
    }

#if TARGET_CARBON
    EndAppModalStateForWindow( (WindowRef) MacGetWindowRef() ) ;
#else
    // TODO probably reenable the parent window if any
    s_macIsInModalLoop = formerModal ;
#endif
}


// Replacement for Show(true) for modal dialogs - returns return code
int wxDialog::ShowModal()
{
    if ( !IsModal() )
    {
        SetModal(true);
    }

    Show(true);
    return GetReturnCode();
}

// NB: this function (surprizingly) may be called for both modal and modeless
//     dialogs and should work for both of them
void wxDialog::EndModal(int retCode)
{
    SetReturnCode(retCode);
    Show(false);
}

// Standard buttons
void wxDialog::OnOK(wxCommandEvent& WXUNUSED(event))
{
  if ( Validate() && TransferDataFromWindow() )
  {
      EndModal(wxID_OK);
  }
}

void wxDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
  if (Validate())
    TransferDataFromWindow();
  // TODO probably need to disable the Apply button until things change again
}

void wxDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void wxDialog::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
    // We'll send a Cancel message by default,
    // which may close the dialog.
    // Check for looping if the Cancel event handler calls Close().

    // Note that if a cancel button and handler aren't present in the dialog,
    // nothing will happen when you close the dialog via the window manager, or
    // via Close().
    // We wouldn't want to destroy the dialog by default, since the dialog may have been
    // created on the stack.
    // However, this does mean that calling dialog->Close() won't delete the dialog
    // unless the handler for wxID_CANCEL does so. So use Destroy() if you want to be
    // sure to destroy the dialog.
    // The default OnCancel (above) simply ends a modal dialog, and hides a modeless dialog.

    static wxList closing;

    if ( closing.Member(this) )
        return;

    closing.Append(this);

    wxCommandEvent cancelEvent(wxEVT_COMMAND_BUTTON_CLICKED, wxID_CANCEL);
    cancelEvent.SetEventObject( this );
    GetEventHandler()->ProcessEvent(cancelEvent); // This may close the dialog

    closing.DeleteObject(this);
}

void wxDialog::OnSysColourChanged(wxSysColourChangedEvent& WXUNUSED(event))
{
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
  Refresh();
}
