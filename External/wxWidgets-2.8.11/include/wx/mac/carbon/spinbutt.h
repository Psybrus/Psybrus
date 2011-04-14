/////////////////////////////////////////////////////////////////////////////
// Name:        spinbutt.h
// Purpose:     wxSpinButton class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: spinbutt.h 35650 2005-09-23 12:56:45Z MR $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SPINBUTT_H_
#define _WX_SPINBUTT_H_

#include "wx/control.h"
#include "wx/event.h"

/*
    The wxSpinButton is like a small scrollbar than is often placed next
    to a text control.

    wxSP_HORIZONTAL:   horizontal spin button
    wxSP_VERTICAL:     vertical spin button (the default)
    wxSP_ARROW_KEYS:   arrow keys increment/decrement value
    wxSP_WRAP:         value wraps at either end
 */

class WXDLLEXPORT wxSpinButton : public wxSpinButtonBase
{
public:
    // construction
    wxSpinButton();

    wxSpinButton(wxWindow *parent,
                 wxWindowID id = -1,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxSP_VERTICAL | wxSP_ARROW_KEYS,
                 const wxString& name = wxT("wxSpinButton"))
    {
        Create(parent, id, pos, size, style, name);
    }

    virtual ~wxSpinButton();

    bool Create(wxWindow *parent,
                wxWindowID id = -1,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxSP_VERTICAL | wxSP_ARROW_KEYS,
                const wxString& name = wxT("wxSpinButton"));


    // accessors
    virtual int GetMin() const;
    virtual int GetMax() const;
    virtual int GetValue() const;
    virtual void SetValue(int val);
    virtual void SetRange(int minVal, int maxVal);

    // implementation
    
    virtual void MacHandleControlClick( WXWidget control , wxInt16 controlpart , bool mouseStillDown ) ;
    virtual wxInt32 MacControlHit( WXEVENTHANDLERREF handler , WXEVENTREF event ) ;

protected:
    void         MacHandleValueChanged( int inc ) ;
   virtual wxSize DoGetBestSize() const;
   int m_value ;

private:
    DECLARE_DYNAMIC_CLASS(wxSpinButton)
};

#endif
    // _WX_SPINBUTT_H_
