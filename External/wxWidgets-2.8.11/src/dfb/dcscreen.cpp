/////////////////////////////////////////////////////////////////////////////
// Name:        src/dfb/dcscreen.cpp
// Purpose:     wxScreenDC implementation
// Author:      Vaclav Slavik
// Created:     2006-08-16
// RCS-ID:      $Id: dcscreen.cpp 54747 2008-07-21 16:58:06Z VZ $
// Copyright:   (c) 2006 REA Elektronik GmbH
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ===========================================================================
// declarations
// ===========================================================================

// ---------------------------------------------------------------------------
// headers
// ---------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "wx/dcscreen.h"

#include "wx/dfb/private.h"

// ===========================================================================
// implementation
// ===========================================================================

//-----------------------------------------------------------------------------
// wxScreenDC
//-----------------------------------------------------------------------------

#warning "FIXME: this doesn't work (neither single app nor multiapp core)"
// FIXME: maybe use a subsurface as well?

IMPLEMENT_DYNAMIC_CLASS(wxScreenDC, wxDC)

wxScreenDC::wxScreenDC()
{
    DFBInit(wxIDirectFB::Get()->GetPrimarySurface());
}

#warning "FIXME: does wxScreenDC need Flip call in dtor?"
