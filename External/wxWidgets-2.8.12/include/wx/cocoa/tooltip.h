///////////////////////////////////////////////////////////////////////////////
// Name:        cocoa/tooltip.h
// Purpose:     wxToolTip class - tooltip control
// Author:      Ryan Norton
// Modified by:
// Created:     31.01.99
// RCS-ID:      $Id: tooltip.h 35698 2005-09-25 20:49:40Z MW $
// Copyright:   (c) Ryan Norton
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COCOA_TOOLTIP_H_
#define _WX_COCOA_TOOLTIP_H_

#include "wx/object.h"

class wxWindow;

class wxToolTip : public wxObject
{
public:
    // ctor & dtor
    wxToolTip(const wxString &tip);
    virtual ~wxToolTip();

    // accessors
        // tip text
    void SetTip(const wxString& tip);
    const wxString& GetTip() const;

        // the window we're associated with
    wxWindow *GetWindow() const;

    // controlling tooltip behaviour: globally change tooltip parameters
        // enable or disable the tooltips globally
    static void Enable(bool flag);
        // set the delay after which the tooltip appears
    static void SetDelay(long milliseconds);

private:
    void SetWindow(wxWindow* window);

    friend class wxWindow;

    wxString  m_text;           // tooltip text
    wxWindow *m_window;         // window we're associated with
    
    DECLARE_ABSTRACT_CLASS(wxToolTip)
};

#endif // _WX_COCOA_TOOLTIP_H_
