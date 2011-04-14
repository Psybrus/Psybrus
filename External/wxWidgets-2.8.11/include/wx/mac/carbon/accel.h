/////////////////////////////////////////////////////////////////////////////
// Name:        accel.h
// Purpose:     wxAcceleratorTable class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: accel.h 42752 2006-10-30 19:26:48Z VZ $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ACCEL_H_
#define _WX_ACCEL_H_

#include "wx/string.h"
#include "wx/event.h"

class WXDLLEXPORT wxAcceleratorTable: public wxObject
{
DECLARE_DYNAMIC_CLASS(wxAcceleratorTable)
public:
    wxAcceleratorTable();
    wxAcceleratorTable(int n, const wxAcceleratorEntry entries[]); // Load from array

    virtual ~wxAcceleratorTable();

    bool Ok() const { return IsOk(); }
    bool IsOk() const;

    int GetCommand( wxKeyEvent &event );
};

// WXDLLEXPORT_DATA(extern wxAcceleratorTable) wxNullAcceleratorTable;

#endif
    // _WX_ACCEL_H_
