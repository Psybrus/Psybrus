/////////////////////////////////////////////////////////////////////////////
// Name:        src/mac/carbon/dcclient.cpp
// Purpose:     wxClientDC class
// Author:      Stefan Csomor
// Modified by:
// Created:     01/02/97
// RCS-ID:      $Id: dcclient.cpp 53969 2008-06-04 04:35:11Z SC $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "wx/dcclient.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/window.h"
    #include "wx/dcmemory.h"
    #include "wx/settings.h"
    #include "wx/toplevel.h"
    #include "wx/math.h"
    #include "wx/region.h"
#endif

#include "wx/graphics.h"
#include "wx/rawbmp.h"
#include "wx/mac/private.h"

//-----------------------------------------------------------------------------
// constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// wxPaintDC
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxWindowDC, wxDC)
IMPLEMENT_DYNAMIC_CLASS(wxClientDC, wxWindowDC)
IMPLEMENT_DYNAMIC_CLASS(wxPaintDC, wxWindowDC)

/*
 * wxWindowDC
 */

#include "wx/mac/uma.h"
#include "wx/notebook.h"
#include "wx/tabctrl.h"


static wxBrush MacGetBackgroundBrush( wxWindow* window )
{
    wxBrush bkdBrush = window->MacGetBackgroundBrush() ;

#if !TARGET_API_MAC_OSX
    // transparency cannot be handled by the OS when not using composited windows
    wxWindow* parent = window->GetParent() ;

    // if we have some 'pseudo' transparency
    if ( ! bkdBrush.Ok() || bkdBrush.GetStyle() == wxTRANSPARENT || window->GetBackgroundColour() == wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE ) )
    {
        // walk up until we find something
        while ( parent != NULL )
        {
            if ( parent->GetBackgroundColour() != wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE ) )
            {
                // if we have any other colours in the hierarchy
                bkdBrush.SetColour( parent->GetBackgroundColour() ) ;
                break ;
            }

            if ( parent->IsKindOf( CLASSINFO(wxTopLevelWindow) ) )
            {
                bkdBrush = parent->MacGetBackgroundBrush() ;
                break ;
            }

            if ( parent->IsKindOf( CLASSINFO( wxNotebook ) )
#if wxUSE_TAB_DIALOG
                 || parent->IsKindOf( CLASSINFO( wxTabCtrl ) )
#endif // wxUSE_TAB_DIALOG
                )
            {
                Rect extent = { 0 , 0 , 0 , 0 } ;
                int x , y ;
                x = y = 0 ;
                wxSize size = parent->GetSize() ;
                parent->MacClientToRootWindow( &x , &y ) ;
                extent.left = x ;
                extent.top = y ;
                extent.top-- ;
                extent.right = x + size.x ;
                extent.bottom = y + size.y ;
                bkdBrush.MacSetThemeBackground( kThemeBackgroundTabPane , (WXRECTPTR) &extent ) ;
                break ;
            }

            parent = parent->GetParent() ;
        }
    }

    if ( !bkdBrush.Ok() || bkdBrush.GetStyle() == wxTRANSPARENT )
    {
        // if we did not find something, use a default
        bkdBrush.MacSetTheme( kThemeBrushDialogBackgroundActive ) ;
    }
#endif

    return bkdBrush ;
}

wxWindowDC::wxWindowDC()
{
    m_window = NULL ;
#if wxMAC_USE_CORE_GRAPHICS
    m_release = false;
#endif
}

wxWindowDC::wxWindowDC(wxWindow *window)
{
    m_window = window ;
    WindowRef rootwindow = (WindowRef) window->MacGetTopLevelWindowRef() ;
    if (!rootwindow)
        return;

    m_ok = true ;

#if wxMAC_USE_CORE_GRAPHICS
    m_window->GetSize( &m_width , &m_height);
    if ( !m_window->IsShownOnScreen() )
        m_width = m_height = 0;
        
    CGContextRef cg = (CGContextRef) window->MacGetCGContextRef();
    m_release = false;
    if ( cg == NULL )
    {
        SetGraphicsContext( wxGraphicsContext::Create( window ) ) ;
    }
    else
    {
        CGContextSaveGState( cg );
        m_release = true ;
        // make sure the context is having its origin at the wx-window coordinates of the
        // view (read at the top of window.cpp about the differences)
        if ( window->MacGetLeftBorderSize() != 0 || window->MacGetTopBorderSize() != 0 )
            CGContextTranslateCTM( cg , -window->MacGetLeftBorderSize() , -window->MacGetTopBorderSize() );

        SetGraphicsContext( wxGraphicsContext::CreateFromNative( cg ) );
    }
    SetClippingRegion( 0 , 0 , m_width , m_height ) ;
#else
    int x , y ;
    x = y = 0 ;
    window->MacWindowToRootWindow( &x , &y ) ;
    m_macLocalOrigin.x = x ;
    m_macLocalOrigin.y = y ;
    m_macPort = UMAGetWindowPort( rootwindow ) ;

    CopyRgn( (RgnHandle) window->MacGetVisibleRegion(true).GetWXHRGN() , (RgnHandle) m_macBoundaryClipRgn ) ;
    OffsetRgn( (RgnHandle) m_macBoundaryClipRgn , m_macLocalOrigin.x , m_macLocalOrigin.y ) ;
    CopyRgn( (RgnHandle) m_macBoundaryClipRgn , (RgnHandle) m_macCurrentClipRgn ) ;
#endif
    SetBackground(MacGetBackgroundBrush(window));

    SetFont( window->GetFont() ) ;
}

wxWindowDC::~wxWindowDC()
{
#if wxMAC_USE_CORE_GRAPHICS
    if ( m_release )
    {
        // this must not necessarily be the current context, we must restore the state of the
        // cg we started with above (before the CGContextTranslateCTM call)
        CGContextRef cg = (CGContextRef) m_window->MacGetCGContextRef();
        CGContextRestoreGState(cg);
    }
#endif
}

void wxWindowDC::DoGetSize( int* width, int* height ) const
{
#if wxMAC_USE_CORE_GRAPHICS
    if ( width )
        *width = m_width;
    if ( height )
        *height = m_height;
#else
    wxCHECK_RET( m_window, _T("GetSize() doesn't work without window") );
    m_window->GetSize(width, height);
#endif
}

wxBitmap wxWindowDC::DoGetAsBitmap(const wxRect *subrect) const
{
#if wxMAC_USE_CORE_GRAPHICS
    if (!m_window)
        return wxNullBitmap;
        
    ControlRef handle = (ControlRef) m_window->GetHandle();
    if ( !handle )
        return wxNullBitmap;

    HIRect rect;
    CGImageRef image;
    CGContextRef context;
    void* data;

    size_t bytesPerRow;

    HIViewCreateOffscreenImage( handle, 0, &rect, &image);

    int width = subrect != NULL ? subrect->width : (int)rect.size.width;
    int height = subrect !=  NULL ? subrect->height : (int)rect.size.height ;

    wxBitmap bmp = wxBitmap(width, height, 32);
    
    context = (CGContextRef)bmp.GetHBITMAP();
    
    CGContextSaveGState(context);
    
    CGContextTranslateCTM( context, 0,  height );
    CGContextScaleCTM( context, 1, -1 );
    
    if ( subrect )
        rect = CGRectOffset( rect, -subrect->x, -subrect->y ) ;
    CGContextDrawImage( context, rect, image );

    CGContextRestoreGState(context);

    return bmp;
#else
    return wxNullBitmap;
#endif
}

/*
 * wxClientDC
 */

wxClientDC::wxClientDC()
{
    m_window = NULL ;
}

#if wxMAC_USE_CORE_GRAPHICS
wxClientDC::wxClientDC(wxWindow *window) :
    wxWindowDC( window )
{
    wxCHECK_RET( window, _T("invalid window in wxClientDC") );
    wxPoint origin = window->GetClientAreaOrigin() ;
    m_window->GetClientSize( &m_width , &m_height);
    if ( !m_window->IsShownOnScreen() )
        m_width = m_height = 0;
    SetDeviceOrigin( origin.x, origin.y );
    SetClippingRegion( 0 , 0 , m_width , m_height ) ;
}
#else
wxClientDC::wxClientDC(wxWindow *window)
{
    wxCHECK_RET( window, _T("invalid window in wxClientDC") );
    m_window = window ;
    wxTopLevelWindowMac* rootwindow = window->MacGetTopLevelWindow() ;
    if (!rootwindow)
        return;

    WindowRef windowref = (WindowRef) rootwindow->MacGetWindowRef() ;
    wxPoint origin = window->GetClientAreaOrigin() ;
    wxSize size = window->GetClientSize() ;
    int x , y ;
    x = origin.x ;
    y = origin.y ;
    window->MacWindowToRootWindow( &x , &y ) ;
    m_macPort = UMAGetWindowPort( windowref ) ;
    m_ok = true ;

    m_macLocalOrigin.x = x ;
    m_macLocalOrigin.y = y ;
    SetRectRgn( (RgnHandle) m_macBoundaryClipRgn , origin.x , origin.y , origin.x + size.x , origin.y + size.y ) ;
    SectRgn( (RgnHandle) m_macBoundaryClipRgn , (RgnHandle) window->MacGetVisibleRegion().GetWXHRGN() , (RgnHandle) m_macBoundaryClipRgn ) ;
    OffsetRgn( (RgnHandle) m_macBoundaryClipRgn , -origin.x , -origin.y ) ;
    OffsetRgn( (RgnHandle) m_macBoundaryClipRgn , m_macLocalOrigin.x , m_macLocalOrigin.y ) ;
    CopyRgn( (RgnHandle) m_macBoundaryClipRgn ,(RgnHandle)  m_macCurrentClipRgn ) ;

    SetBackground(MacGetBackgroundBrush(window));
    SetFont( window->GetFont() ) ;
}
#endif

wxClientDC::~wxClientDC()
{
}

#if !wxMAC_USE_CORE_GRAPHICS
void wxClientDC::DoGetSize(int *width, int *height) const
{
    wxCHECK_RET( m_window, _T("GetSize() doesn't work without window") );

    m_window->GetClientSize( width, height );
}
#endif

/*
 * wxPaintDC
 */

wxPaintDC::wxPaintDC()
{
    m_window = NULL ;
}

#if wxMAC_USE_CORE_GRAPHICS
wxPaintDC::wxPaintDC(wxWindow *window) :
    wxWindowDC( window )
{
    wxPoint origin = window->GetClientAreaOrigin() ;
    m_window->GetClientSize( &m_width , &m_height);
    SetDeviceOrigin( origin.x, origin.y );
    SetClippingRegion( 0 , 0 , m_width , m_height ) ;
}
#else
wxPaintDC::wxPaintDC(wxWindow *window)
{
    m_window = window ;
    wxTopLevelWindowMac* rootwindow = window->MacGetTopLevelWindow() ;
    WindowRef windowref = (WindowRef) rootwindow->MacGetWindowRef() ;
    wxPoint origin = window->GetClientAreaOrigin() ;
    wxSize size = window->GetClientSize() ;
    int x , y ;
    x = origin.x ;
    y = origin.y ;
    window->MacWindowToRootWindow( &x , &y ) ;
    m_macPort = UMAGetWindowPort( windowref ) ;
    m_ok = true ;

#if wxMAC_USE_CORE_GRAPHICS
    if ( window->MacGetCGContextRef() )
    {
        m_graphicContext = new wxMacCGContext( (CGContextRef) window->MacGetCGContextRef() ) ;
        m_graphicContext->SetPen( m_pen ) ;
        m_graphicContext->SetBrush( m_brush ) ;
        SetClippingRegion( 0 , 0 , size.x , size.y ) ;
        SetBackground(MacGetBackgroundBrush(window));
    }
    else
    {
        wxLogDebug(wxT("You cannot create a wxPaintDC outside an OS-draw event") ) ;
        m_graphicContext = NULL ;
    }
    // there is no out-of-order drawing on OSX
#else
    m_macLocalOrigin.x = x ;
    m_macLocalOrigin.y = y ;
    SetRectRgn( (RgnHandle) m_macBoundaryClipRgn , origin.x , origin.y , origin.x + size.x , origin.y + size.y ) ;
    SectRgn( (RgnHandle) m_macBoundaryClipRgn , (RgnHandle) window->MacGetVisibleRegion().GetWXHRGN() , (RgnHandle) m_macBoundaryClipRgn ) ;
    OffsetRgn( (RgnHandle) m_macBoundaryClipRgn , -origin.x , -origin.y ) ;
    SectRgn( (RgnHandle) m_macBoundaryClipRgn  , (RgnHandle) window->GetUpdateRegion().GetWXHRGN() , (RgnHandle) m_macBoundaryClipRgn ) ;
    OffsetRgn( (RgnHandle) m_macBoundaryClipRgn , m_macLocalOrigin.x , m_macLocalOrigin.y ) ;
    CopyRgn( (RgnHandle) m_macBoundaryClipRgn , (RgnHandle) m_macCurrentClipRgn ) ;
    SetBackground(MacGetBackgroundBrush(window));
#endif

    SetFont( window->GetFont() ) ;
}
#endif

wxPaintDC::~wxPaintDC()
{
}

#if !wxMAC_USE_CORE_GRAPHICS
void wxPaintDC::DoGetSize(int *width, int *height) const
{
    wxCHECK_RET( m_window, _T("GetSize() doesn't work without window") );

    m_window->GetClientSize( width, height );
}
#endif
