/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/colordlg.h
// Purpose:     wxColourDialog
// Author:      Vaclav Slavik
// Modified by:
// Created:     2004/06/04
// RCS-ID:      $Id: colordlg.h 40923 2006-08-30 05:55:56Z PC $
// Copyright:   (c) Vaclav Slavik, 2004
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_COLORDLG_H_
#define _WX_GTK_COLORDLG_H_

#include "wx/dialog.h"
#include "wx/cmndata.h"

class WXDLLEXPORT wxColourDialog : public wxDialog
{
public:
    wxColourDialog() {}
    wxColourDialog(wxWindow *parent,
                   wxColourData *data = (wxColourData *)NULL);
    virtual ~wxColourDialog() {}

    bool Create(wxWindow *parent, wxColourData *data = (wxColourData *)NULL);

    wxColourData &GetColourData() { return m_data; }

    virtual int ShowModal();

protected:
    // implement some base class methods to do nothing to avoid asserts and
    // GTK warnings, since this is not a real wxDialog.
    virtual void DoSetSize(int WXUNUSED(x), int WXUNUSED(y),
                           int WXUNUSED(width), int WXUNUSED(height),
                           int WXUNUSED(sizeFlags) = wxSIZE_AUTO) {}
    virtual void DoMoveWindow(int WXUNUSED(x), int WXUNUSED(y),
                              int WXUNUSED(width), int WXUNUSED(height)) {}

    // copy data between the dialog and m_colourData:
    void ColourDataToDialog();
    void DialogToColourData();

    wxColourData m_data;

    DECLARE_DYNAMIC_CLASS(wxColourDialog)
};

#endif
