/////////////////////////////////////////////////////////////////////////////
// Name:        wx/mac/classic/colordlg.h
// Purpose:     wxColourDialog class. Use generic version if no
//              platform-specific implementation.
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: colordlg.h 37164 2006-01-26 17:20:50Z ABX $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COLORDLG_H_
#define _WX_COLORDLG_H_

#include "wx/defs.h"
#include "wx/dialog.h"
#include "wx/cmndata.h"

/*
 * Platform-specific colour dialog implementation
 */

class WXDLLEXPORT wxColourDialog: public wxDialog
{
DECLARE_DYNAMIC_CLASS(wxColourDialog)
public:
    wxColourDialog();
    wxColourDialog(wxWindow *parent, wxColourData *data = NULL);

    bool Create(wxWindow *parent, wxColourData *data = NULL);

    int ShowModal();
    wxColourData& GetColourData() { return m_colourData; }

protected:
    wxColourData        m_colourData;
    wxWindow*           m_dialogParent;
};

#endif
    // _WX_COLORDLG_H_
