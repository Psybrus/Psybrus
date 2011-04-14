/////////////////////////////////////////////////////////////////////////////
// Name:        src/dfb/dcmemory.cpp
// Purpose:     wxMemoryDC implementation
// Author:      Vaclav Slavik
// Created:     2006-08-16
// RCS-ID:      $Id: dcmemory.cpp 42783 2006-10-30 22:42:34Z VZ $
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

#include "wx/dcmemory.h"

#ifndef WX_PRECOMP
    #include "wx/bitmap.h"
#endif

#include "wx/dfb/private.h"

// ===========================================================================
// implementation
// ===========================================================================

//-----------------------------------------------------------------------------
// wxMemoryDC
//-----------------------------------------------------------------------------

#warning "FIXME: verify/fix that wxMemoryDC works correctly with mono bitmaps"

IMPLEMENT_DYNAMIC_CLASS(wxMemoryDC, wxDC)

void wxMemoryDC::Init()
{
}

wxMemoryDC::wxMemoryDC(wxDC *WXUNUSED(dc))
{
}

void wxMemoryDC::DoSelect(const wxBitmap& bitmap)
{
    m_bmp = bitmap;

    if ( !bitmap.Ok() )
    {
        // select the bitmap out of the DC
        m_surface = NULL;
        return;
    }

    // init the DC for drawing to this bitmap
    DFBInit(bitmap.GetDirectFBSurface());
}
