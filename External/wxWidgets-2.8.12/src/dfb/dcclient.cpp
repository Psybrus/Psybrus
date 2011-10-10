/////////////////////////////////////////////////////////////////////////////
// Name:        src/dfb/dcclient.cpp
// Purpose:     wxWindowDC, wxClientDC and wxPaintDC
// Author:      Vaclav Slavik
// Created:     2006-08-10
// RCS-ID:      $Id: dcclient.cpp 54748 2008-07-21 17:01:35Z VZ $
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

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/dcclient.h"

#ifndef WX_PRECOMP
    #include "wx/toplevel.h"
    #include "wx/window.h"
#endif

#include "wx/dfb/private.h"

#define TRACE_PAINT  _T("paint")

// ===========================================================================
// implementation
// ===========================================================================

//-----------------------------------------------------------------------------
// helpers
//-----------------------------------------------------------------------------

// Returns subrect of the window that is not outside of its parent's
// boundaries ("hidden behind its borders"), recursively:
static wxRect GetUncoveredWindowArea(wxWindow *win)
{
    wxRect r(win->GetSize());

    if ( win->IsTopLevel() )
        return r;

    wxWindow *parent = win->GetParent();
    if ( !parent )
        return r;

    // intersect with parent's uncovered area, after offsetting it into win's
    // coordinates; this will remove parts of 'r' that are outside of the
    // parent's area:
    wxRect rp(GetUncoveredWindowArea(parent));
    rp.Offset(-win->GetPosition());
    rp.Offset(-parent->GetClientAreaOrigin());
    r.Intersect(rp);

    return r;
}

// creates a dummy surface that has the same format as the real window's
// surface, but is not visible and so can be painted on even if the window
// is hidden
static
wxIDirectFBSurfacePtr CreateDummySurface(wxWindow *win, const wxRect *rect)
{
    wxLogTrace(TRACE_PAINT, _T("%p ('%s'): creating dummy DC surface"),
               win, win->GetName().c_str());
    wxSize size(rect ? rect->GetSize() : win->GetSize());
    return win->GetDfbSurface()->CreateCompatible
           (
             size,
             wxIDirectFBSurface::CreateCompatible_NoBackBuffer
           );
}

//-----------------------------------------------------------------------------
// wxWindowDC
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxWindowDC, wxDC)

wxWindowDC::wxWindowDC(wxWindow *win)
{
    InitForWin(win, NULL);
}

void wxWindowDC::InitForWin(wxWindow *win, const wxRect *rect)
{
    wxCHECK_RET( win, _T("invalid window") );

    m_win = win;

    // obtain the surface used for painting:
    wxPoint origin;
    wxIDirectFBSurfacePtr surface;

    wxRect rectOrig(rect ? *rect : wxRect(win->GetSize()));
    wxRect r;

    if ( !win->IsShownOnScreen() )
    {
        // leave 'r' rectangle empty to indicate the window is not visible,
        // see below (below "create the surface:") for how is this case handled
    }
    else
    {
        // compute painting rectangle after clipping if we're in PaintWindow
        // code, otherwise paint on the entire window:
        r = rectOrig;

        const wxRegion& updateRegion = win->GetUpdateRegion();
        if ( win->GetTLW()->IsPainting() && !updateRegion.IsEmpty() )
        {
            r.Intersect(updateRegion.AsRect());
            wxCHECK_RET( !r.IsEmpty(), _T("invalid painting rectangle") );

            // parent TLW will flip the entire surface when painting is done
            m_shouldFlip = false;
        }
        else
        {
            // One of two things happened:
            // (1) the TLW is not being painted by PaintWindow() now; or
            // (2) we're drawing on some window other than the one that is
            //     currently painted on by PaintWindow()
            // In either case, we need to flip the surface when we're done
            // painting and we don't have to use updateRegion for clipping.
            // OTOH, if the window is (partially) hidden by being
            // out of its parent's area, we must clip the surface accordingly.
            r.Intersect(GetUncoveredWindowArea(win));
            m_shouldFlip = true; // paint the results immediately
        }
    }

    // create the surface:
    if ( r.IsEmpty() )
    {
        // we're painting on invisible window: the changes won't have any
        // effect, as the window will be repainted anyhow when it is shown,
        // but we still need a valid DC so that e.g. text extents can be
        // measured, so let's create a dummy surface that has the same
        // format as the real one would have and let the code paint on it:
        surface = CreateDummySurface(win, rect);

        // painting on hidden window has no effect on TLW's surface, don't
        // waste time flipping the dummy surface:
        m_shouldFlip = false;
    }
    else
    {
        m_winRect = r;
        DFBRectangle dfbrect = { r.x, r.y, r.width, r.height };
        surface = win->GetDfbSurface()->GetSubSurface(&dfbrect);

        // if the DC was clipped thanks to rectPaint, we must adjust the
        // origin accordingly; but we do *not* adjust for 'rect', because
        // rect.GetPosition() has coordinates (0,0) in the DC:
        origin.x = rectOrig.x - r.x;
        origin.y = rectOrig.y - r.y;

        // m_shouldFlip was set in the "if" block above this one
    }

    if ( !surface )
        return;

    wxLogTrace(TRACE_PAINT,
               _T("%p ('%s'): creating DC for area [%i,%i,%i,%i], clipped to [%i,%i,%i,%i], origin [%i,%i]"),
               win, win->GetName().c_str(),
               rectOrig.x, rectOrig.y, rectOrig.GetRight(), rectOrig.GetBottom(),
               r.x, r.y, r.GetRight(), r.GetBottom(),
               origin.x, origin.y);

    DFBInit(surface);
    SetFont(win->GetFont());

    // offset coordinates to account for subsurface's origin coordinates:
    SetDeviceOrigin(origin.x, origin.y);
}

wxWindowDC::~wxWindowDC()
{
    wxIDirectFBSurfacePtr surface(GetDirectFBSurface());
    if ( !surface )
        return;

    // if no painting was done on the DC, we don't have to flip the surface:
    if ( !m_isBBoxValid )
        return;

    if ( m_shouldFlip )
    {
        // paint overlays on top of the surface being drawn to by this DC
        // before showing anything on the screen:
        m_win->PaintOverlays(m_winRect);

        DFBSurfaceCapabilities caps = DSCAPS_NONE;
        surface->GetCapabilities(&caps);
        if ( caps & DSCAPS_DOUBLE )
        {
            // FIXME: flip only modified parts of the surface
            surface->FlipToFront();
        }
        // else: the surface is not double-buffered and so cannot be flipped
    }
    // else: don't flip the surface, wxTLW will do it when it finishes
    //       painting of its invalidated areas
}

//-----------------------------------------------------------------------------
// wxClientDC
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxClientDC, wxWindowDC)

wxClientDC::wxClientDC(wxWindow *win)
{
    wxCHECK_RET( win, _T("invalid window") );

    wxRect rect = win->GetClientRect();
    InitForWin(win, &rect);
}

//-----------------------------------------------------------------------------
// wxPaintDC
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxPaintDC, wxWindowDC)
