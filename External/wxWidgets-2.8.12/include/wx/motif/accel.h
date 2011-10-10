/////////////////////////////////////////////////////////////////////////////
// Name:        wx/motif/accel.h
// Purpose:     wxAcceleratorTable class
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// RCS-ID:      $Id: accel.h 42752 2006-10-30 19:26:48Z VZ $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ACCEL_H_
#define _WX_ACCEL_H_

#include "wx/object.h"
#include "wx/string.h"
#include "wx/event.h"

class WXDLLEXPORT wxAcceleratorTable: public wxObject
{
    DECLARE_DYNAMIC_CLASS(wxAcceleratorTable)
public:
    wxAcceleratorTable();
    wxAcceleratorTable(const wxString& resource); // Load from .rc resource
    wxAcceleratorTable(int n, const wxAcceleratorEntry entries[]); // Load from array

    virtual ~wxAcceleratorTable();

    bool Ok() const { return IsOk(); }
    bool IsOk() const;

    // Implementation only
    int GetCount() const;
    wxAcceleratorEntry* GetEntries() const;
};

WXDLLEXPORT_DATA(extern wxAcceleratorTable) wxNullAcceleratorTable;

#endif
// _WX_ACCEL_H_
