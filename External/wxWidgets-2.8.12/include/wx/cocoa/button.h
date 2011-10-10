/////////////////////////////////////////////////////////////////////////////
// Name:        wx/cocoa/button.h
// Purpose:     wxButton class
// Author:      David Elliott
// Modified by:
// Created:     2002/12/29
// RCS-ID:      $Id: button.h 48529 2007-09-03 17:17:35Z DE $
// Copyright:   (c) 2002 David Elliott
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_COCOA_BUTTON_H__
#define __WX_COCOA_BUTTON_H__

#include "wx/cocoa/NSButton.h"

// ========================================================================
// wxButton
// ========================================================================
class WXDLLEXPORT wxButton : public wxButtonBase, protected wxCocoaNSButton
{
    DECLARE_DYNAMIC_CLASS(wxButton)
    DECLARE_EVENT_TABLE()
    WX_DECLARE_COCOA_OWNER(NSButton,NSControl,NSView)
// ------------------------------------------------------------------------
// initialization
// ------------------------------------------------------------------------
public:
   wxButton() { }
   wxButton(wxWindow *parent, wxWindowID winid,
             const wxString& label = wxEmptyString,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize, long style = 0,
             const wxValidator& validator = wxDefaultValidator,
             const wxString& name = wxButtonNameStr)
    {
        Create(parent, winid, label, pos, size, style, validator, name);
    }


    bool Create(wxWindow *parent, wxWindowID winid,
            const wxString& label = wxEmptyString,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxButtonNameStr);

    virtual ~wxButton();

// ------------------------------------------------------------------------
// Cocoa callbacks
// ------------------------------------------------------------------------
protected:
    virtual void Cocoa_wxNSButtonAction(void);
// ------------------------------------------------------------------------
// Implementation
// ------------------------------------------------------------------------
public:
    wxString GetLabel() const;
    void SetLabel(const wxString& label);
    wxSize DoGetBestSize() const;
    virtual void SetDefault();
};

#endif
    // __WX_COCOA_BUTTON_H__
