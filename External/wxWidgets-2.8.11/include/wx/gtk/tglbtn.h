/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/tglbtn.h
// Purpose:     Declaration of the wxToggleButton class, which implements a
//              toggle button under wxGTK.
// Author:      John Norris, minor changes by Axel Schlueter
// Modified by:
// Created:     08.02.01
// RCS-ID:      $Id: tglbtn.h 40815 2006-08-25 12:59:28Z VZ $
// Copyright:   (c) 2000 Johnny C. Norris II
// License:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_TOGGLEBUTTON_H_
#define _WX_GTK_TOGGLEBUTTON_H_

#include "wx/bitmap.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxToggleButton;
class WXDLLIMPEXP_CORE wxToggleBitmapButton;

//-----------------------------------------------------------------------------
// global data
//-----------------------------------------------------------------------------

extern WXDLLIMPEXP_CORE const wxChar wxCheckBoxNameStr[];

//-----------------------------------------------------------------------------
// wxToggleBitmapButton
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxToggleBitmapButton: public wxControl
{
public:
    // construction/destruction
    wxToggleBitmapButton() {}
    wxToggleBitmapButton(wxWindow *parent,
                   wxWindowID id,
                   const wxBitmap& label,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxValidator& validator = wxDefaultValidator,
                   const wxString& name = wxCheckBoxNameStr)
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    // Create the control
    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxBitmap& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxCheckBoxNameStr);

    // Get/set the value
    void SetValue(bool state);
    bool GetValue() const;

    // Set the label
    virtual void SetLabel(const wxString& label) { wxControl::SetLabel(label); }
    virtual void SetLabel(const wxBitmap& label);
    bool Enable(bool enable = TRUE);

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWINDOW_VARIANT_NORMAL);

    // implementation
    bool      m_blockEvent;
    wxBitmap  m_bitmap;

    void OnSetBitmap();

protected:
    virtual wxSize DoGetBestSize() const;
    virtual void DoApplyWidgetStyle(GtkRcStyle *style);
    virtual GdkWindow *GTKGetWindow(wxArrayGdkWindows& windows) const;

private:
    DECLARE_DYNAMIC_CLASS(wxToggleBitmapButton)
};

//-----------------------------------------------------------------------------
// wxToggleButton
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxToggleButton: public wxControl
{
public:
    // construction/destruction
    wxToggleButton() {}
    wxToggleButton(wxWindow *parent,
                   wxWindowID id,
                   const wxString& label,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxValidator& validator = wxDefaultValidator,
                   const wxString& name = wxCheckBoxNameStr)
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    // Create the control
    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxCheckBoxNameStr);

    // Get/set the value
    void SetValue(bool state);
    bool GetValue() const;

    // Set the label
    void SetLabel(const wxString& label);
    bool Enable(bool enable = TRUE);

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWINDOW_VARIANT_NORMAL);
 
    // implementation
    bool m_blockEvent;

protected:
    virtual wxSize DoGetBestSize() const;
    virtual void DoApplyWidgetStyle(GtkRcStyle *style);
    virtual GdkWindow *GTKGetWindow(wxArrayGdkWindows& windows) const;

private:
    DECLARE_DYNAMIC_CLASS(wxToggleButton)
};

#endif // _WX_GTK_TOGGLEBUTTON_H_

