/////////////////////////////////////////////////////////////////////////////
// Name:        plot.cpp
// Purpose:     wxPlotWindow
// Author:      Robert Roebling
// Modified by:
// Created:     12/01/2000
// RCS-ID:      $Id: plot.cpp 36417 2005-12-17 19:23:04Z VZ $
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/object.h"
#include "wx/font.h"
#include "wx/colour.h"
#include "wx/settings.h"
#include "wx/sizer.h"
#include "wx/log.h"
#include "wx/intl.h"
#include "wx/dcclient.h"
#include "wx/stattext.h"
#endif

#include "wx/plot/plot.h"
#include "wx/bmpbuttn.h"
#include "wx/module.h"

#include <math.h>

// ----------------------------------------------------------------------------
// XPMs
// ----------------------------------------------------------------------------

#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "wx/plot/plot_enl.xpm"
    #include "wx/plot/plot_shr.xpm"
    #include "wx/plot/plot_zin.xpm"
    #include "wx/plot/plot_zot.xpm"
    #include "wx/plot/plot_up.xpm"
    #include "wx/plot/plot_dwn.xpm"
#endif

//----------------------------------------------------------------------------
// event types
//----------------------------------------------------------------------------

DEFINE_EVENT_TYPE(wxEVT_PLOT_SEL_CHANGING)
DEFINE_EVENT_TYPE(wxEVT_PLOT_SEL_CHANGED)
DEFINE_EVENT_TYPE(wxEVT_PLOT_CLICKED)
DEFINE_EVENT_TYPE(wxEVT_PLOT_DOUBLECLICKED)
DEFINE_EVENT_TYPE(wxEVT_PLOT_ZOOM_IN)
DEFINE_EVENT_TYPE(wxEVT_PLOT_ZOOM_OUT)
DEFINE_EVENT_TYPE(wxEVT_PLOT_VALUE_SEL_CREATING)
DEFINE_EVENT_TYPE(wxEVT_PLOT_VALUE_SEL_CREATED)
DEFINE_EVENT_TYPE(wxEVT_PLOT_VALUE_SEL_CHANGING)
DEFINE_EVENT_TYPE(wxEVT_PLOT_VALUE_SEL_CHANGED)
DEFINE_EVENT_TYPE(wxEVT_PLOT_AREA_SEL_CREATING)
DEFINE_EVENT_TYPE(wxEVT_PLOT_AREA_SEL_CREATED)
DEFINE_EVENT_TYPE(wxEVT_PLOT_AREA_SEL_CHANGING)
DEFINE_EVENT_TYPE(wxEVT_PLOT_AREA_SEL_CHANGED)
DEFINE_EVENT_TYPE(wxEVT_PLOT_BEGIN_X_LABEL_EDIT)
DEFINE_EVENT_TYPE(wxEVT_PLOT_END_X_LABEL_EDIT)
DEFINE_EVENT_TYPE(wxEVT_PLOT_BEGIN_Y_LABEL_EDIT)
DEFINE_EVENT_TYPE(wxEVT_PLOT_END_Y_LABEL_EDIT)
DEFINE_EVENT_TYPE(wxEVT_PLOT_BEGIN_TITLE_EDIT)
DEFINE_EVENT_TYPE(wxEVT_PLOT_END_TITLE_EDIT)
DEFINE_EVENT_TYPE(wxEVT_PLOT_AREA_CREATE)

//----------------------------------------------------------------------------
// accessor functions for the bitmaps (may return NULL, check for it!)
//----------------------------------------------------------------------------

static wxBitmap *GetEnlargeBitmap();
static wxBitmap *GetShrinkBitmap();
static wxBitmap *GetZoomInBitmap();
static wxBitmap *GetZoomOutBitmap();
static wxBitmap *GetUpBitmap();
static wxBitmap *GetDownBitmap();

//-----------------------------------------------------------------------------
// consts
//-----------------------------------------------------------------------------

#define wxPLOT_SCROLL_STEP  30

//-----------------------------------------------------------------------------
// wxPlotEvent
//-----------------------------------------------------------------------------

wxPlotEvent::wxPlotEvent( wxEventType commandType, int id )
    : wxNotifyEvent( commandType, id )
{
    m_curve = (wxPlotCurve*) NULL;
    m_zoom = 1.0;
    m_position = 0;
}

//-----------------------------------------------------------------------------
// wxPlotCurve
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(wxPlotCurve, wxObject)

wxPlotCurve::wxPlotCurve( int offsetY, double startY, double endY )
: m_penNormal(*wxGREY_PEN), m_penSelected(*wxBLACK_PEN)
{
    m_offsetY = offsetY;
    m_startY = startY;
    m_endY = endY;
}

//-----------------------------------------------------------------------------
// wxPlotOnOffCurve
//-----------------------------------------------------------------------------

IMPLEMENT_CLASS(wxPlotOnOffCurve, wxObject)

#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY(wxArrayPlotOnOff);

wxPlotOnOffCurve::wxPlotOnOffCurve( int offsetY )
{
    m_offsetY = offsetY;
    m_minX = -1;
    m_maxX = -1;
}

void wxPlotOnOffCurve::Add( wxInt32 on, wxInt32 off, void *clientData )
{
    wxASSERT_MSG( on > 0, _T("plot index < 0") );
    wxASSERT( on <= off );

    if (m_minX == -1)
        m_minX = on;
    if (off > m_maxX)
        m_maxX = off;

    wxPlotOnOff *v = new wxPlotOnOff;
    v->m_on = on;
    v->m_off = off;
    v->m_clientData = clientData;
    m_marks.Add( v );
}

size_t wxPlotOnOffCurve::GetCount()
{
    return m_marks.GetCount();
}

wxInt32 wxPlotOnOffCurve::GetOn( size_t index )
{
    wxPlotOnOff *v = &m_marks.Item( index );
    return v->m_on;
}

wxInt32 wxPlotOnOffCurve::GetOff( size_t index )
{
    wxPlotOnOff *v = &m_marks.Item( index );
    return v->m_off;
}

void* wxPlotOnOffCurve::GetClientData( size_t index )
{
    wxPlotOnOff *v = &m_marks.Item( index );
    return v->m_clientData;
}

wxPlotOnOff *wxPlotOnOffCurve::GetAt( size_t index )
{
    return &m_marks.Item( index );
}

void wxPlotOnOffCurve::DrawOnLine( wxDC &dc, wxCoord y, wxCoord start, wxCoord end, void *WXUNUSED(clientData) )
{
    dc.DrawLine( start, y, start, y-30 );
    dc.DrawLine( start, y-30, end, y-30 );
    dc.DrawLine( end, y-30, end, y );
}

void wxPlotOnOffCurve::DrawOffLine( wxDC &dc, wxCoord y, wxCoord start, wxCoord end )
{
    dc.DrawLine( start, y, end, y );
}

//-----------------------------------------------------------------------------
// wxPlotArea
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxPlotArea, wxWindow)

BEGIN_EVENT_TABLE(wxPlotArea, wxWindow)
    EVT_PAINT(        wxPlotArea::OnPaint)
    EVT_LEFT_DOWN(    wxPlotArea::OnMouse)
    EVT_LEFT_DCLICK(  wxPlotArea::OnMouse)
END_EVENT_TABLE()

wxPlotArea::wxPlotArea( wxPlotWindow *parent )
        : wxWindow( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("plotarea") )
{
    m_owner = parent;

    m_zooming = false;

    SetBackgroundColour( *wxWHITE );
}

void wxPlotArea::OnMouse( wxMouseEvent &event )
{
    int client_width;
    int client_height;
    GetClientSize( &client_width, &client_height);
    int view_x;
    int view_y;
    m_owner->GetViewStart( &view_x, &view_y );
    view_x *= wxPLOT_SCROLL_STEP;
    view_y *= wxPLOT_SCROLL_STEP;

    wxCoord x = event.GetX();
    wxCoord y = event.GetY();
    x += view_x;
    y += view_y;

    wxList::compatibility_iterator node = m_owner->m_curves.GetFirst();
    while (node)
    {
        wxPlotCurve *curve = (wxPlotCurve*)node->GetData();

        double double_client_height = (double)client_height;
        double range = curve->GetEndY() - curve->GetStartY();
        double end = curve->GetEndY();
        wxCoord offset_y = curve->GetOffsetY();

        double dy = (end - curve->GetY( (wxInt32)(x/m_owner->GetZoom()) )) / range;
        wxCoord curve_y = (wxCoord)(dy * double_client_height) - offset_y - 1;

        if ((y-curve_y < 4) && (y-curve_y > -4))
        {
            wxPlotEvent event1( event.ButtonDClick() ? wxEVT_PLOT_DOUBLECLICKED : wxEVT_PLOT_CLICKED, m_owner->GetId() );
            event1.SetEventObject( m_owner );
            event1.SetZoom( m_owner->GetZoom() );
            event1.SetCurve( curve );
            event1.SetPosition( (int)floor(x/m_owner->GetZoom()) );
            m_owner->GetEventHandler()->ProcessEvent( event1 );

            if (curve != m_owner->GetCurrentCurve())
            {
                wxPlotEvent event2( wxEVT_PLOT_SEL_CHANGING, m_owner->GetId() );
                event2.SetEventObject( m_owner );
                event2.SetZoom( m_owner->GetZoom() );
                event2.SetCurve( curve );
                if (!m_owner->GetEventHandler()->ProcessEvent( event2 ) || event2.IsAllowed())
                {
                    m_owner->SetCurrentCurve( curve );
                }
            }
            return;
        }

        node = node->GetNext();
    }
}

void wxPlotArea::DeleteCurve( wxPlotCurve *curve, int from, int to )
{
    wxClientDC dc(this);
    m_owner->PrepareDC( dc );
    dc.SetPen( *wxWHITE_PEN );
    DrawCurve( &dc, curve, from, to );
}

void wxPlotArea::DrawCurve( wxDC *dc, wxPlotCurve *curve, int from, int to )
{
    int view_x;
    int view_y;
    m_owner->GetViewStart( &view_x, &view_y );
    view_x *= wxPLOT_SCROLL_STEP;

    if (from == -1)
        from = view_x;

    int client_width;
    int client_height;
    GetClientSize( &client_width, &client_height);

    if (to == -1)
        to = view_x + client_width;

    double zoom = m_owner->GetZoom();

    int start_x = wxMax( from, (int)floor(curve->GetStartX()*zoom) );
    int end_x = wxMin( to, (int)floor(curve->GetEndX()*zoom) );

    start_x = wxMax( view_x, start_x );
    end_x = wxMin( view_x + client_width, end_x );

    end_x++;

    double double_client_height = (double)client_height;
    double range = curve->GetEndY() - curve->GetStartY();
    double end = curve->GetEndY();
    wxCoord offset_y = curve->GetOffsetY();

    wxCoord last_y=0;
    for (int x = start_x; x < end_x; x++)
    {
        double dy = (end - curve->GetY( (wxInt32)(x/zoom) )) / range;
        wxCoord y = (wxCoord)(dy * double_client_height) - offset_y - 1;

        if (x != start_x)
           dc->DrawLine( x-1, last_y, x, y );

        last_y = y;
    }
}

void wxPlotArea::DrawOnOffCurve( wxDC *dc, wxPlotOnOffCurve *curve, int from, int to )
{
    int view_x;
    int view_y;
    m_owner->GetViewStart( &view_x, &view_y );
    view_x *= wxPLOT_SCROLL_STEP;

    if (from == -1)
        from = view_x;

    int client_width;
    int client_height;
    GetClientSize( &client_width, &client_height);

    if (to == -1)
        to = view_x + client_width;

    double zoom = m_owner->GetZoom();

    int start_x = wxMax( from, (int)floor(curve->GetStartX()*zoom) );
    int end_x = wxMin( to, (int)floor(curve->GetEndX()*zoom) );

    start_x = wxMax( view_x, start_x );
    end_x = wxMin( view_x + client_width, end_x );

    end_x++;

    wxCoord offset_y = curve->GetOffsetY();
    wxCoord last_off = -5;

    if (curve->GetCount() == 0)
        return;

    for (size_t index = 0; index < curve->GetCount(); index++)
    {
        wxPlotOnOff *p = curve->GetAt( index );

        wxCoord on = (wxCoord)(p->m_on*zoom);
        wxCoord off = (wxCoord)(p->m_off*zoom);

        if (end_x < on)
        {
            curve->DrawOffLine( *dc, client_height-offset_y, last_off, on );
            break;
        }

        if (off >= start_x)
        {
            curve->DrawOffLine( *dc, client_height-offset_y, last_off, on );
            curve->DrawOnLine( *dc, client_height-offset_y, on, off, p->m_clientData );
        }
        last_off = off;
    }

    wxPlotOnOff *p = curve->GetAt( curve->GetCount()-1 );
    wxCoord off = (wxCoord)(p->m_off*zoom);
    if (off < end_x)
        curve->DrawOffLine( *dc, client_height-offset_y, off, to );
}

void wxPlotArea::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    int view_x;
    int view_y;
    m_owner->GetViewStart( &view_x, &view_y );
    view_x *= wxPLOT_SCROLL_STEP;
    view_y *= wxPLOT_SCROLL_STEP;

    wxPaintDC dc( this );
    m_owner->PrepareDC( dc );

    wxRegionIterator upd( GetUpdateRegion() );

    while (upd)
    {
        int update_x = upd.GetX() + view_x;
        int update_width = upd.GetWidth();

/*
        if (m_owner->m_current)
        {
            dc.SetPen( *wxLIGHT_GREY_PEN );
            int base_line = client_height - m_owner->m_current->GetOffsetY();
            dc.DrawLine( update_x-1, base_line-1, update_x+update_width+2, base_line-1 );
        }
*/

        wxList::compatibility_iterator node = m_owner->m_curves.GetFirst();
        while (node)
        {
            wxPlotCurve *curve = (wxPlotCurve*) node->GetData();

            if (curve == m_owner->GetCurrentCurve())
                dc.SetPen( curve->GetPenSelected() );
            else
                dc.SetPen( curve->GetPenNormal() );

            DrawCurve( &dc, curve, update_x-1, update_x+update_width+2 );

            node = node->GetNext();
        }

        dc.SetPen( *wxRED_PEN );

        node = m_owner->m_onOffCurves.GetFirst();
        while (node)
        {
            wxPlotOnOffCurve *curve = (wxPlotOnOffCurve*) node->GetData();

            DrawOnOffCurve( &dc, curve, update_x-1, update_x+update_width+2 );

            node = node->GetNext();
        }

        upd ++;
    }
}

void wxPlotArea::ScrollWindow( int dx, int dy, const wxRect *rect )
{
    wxWindow::ScrollWindow( dx, dy, rect );
//    m_owner->m_xaxis->ScrollWindow( dx, 0 );
}

//-----------------------------------------------------------------------------
// wxPlotXAxisArea
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxPlotXAxisArea, wxWindow)

BEGIN_EVENT_TABLE(wxPlotXAxisArea, wxWindow)
  EVT_PAINT(        wxPlotXAxisArea::OnPaint)
  EVT_LEFT_DOWN(    wxPlotXAxisArea::OnMouse)
END_EVENT_TABLE()

wxPlotXAxisArea::wxPlotXAxisArea( wxPlotWindow *parent )
        : wxWindow( parent, wxID_ANY, wxDefaultPosition, wxSize(wxDefaultCoord,40), 0, _T("plotxaxisarea") )
{
    m_owner = parent;

    SetBackgroundColour( *wxWHITE );
    SetFont( *wxSMALL_FONT );
}

void wxPlotXAxisArea::OnMouse( wxMouseEvent &event )
{
    int client_width;
    int client_height;
    GetClientSize( &client_width, &client_height);
    int view_x;
    int view_y;
    m_owner->GetViewStart( &view_x, &view_y );
    view_x *= wxPLOT_SCROLL_STEP;
    view_y *= wxPLOT_SCROLL_STEP;

    wxCoord x = event.GetX() + view_x;
    wxCoord y = event.GetY() + view_y;

    /* TO DO: do something here */
    wxUnusedVar(x);
    wxUnusedVar(y);
}

void wxPlotXAxisArea::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    int view_x;
    int view_y;
    m_owner->GetViewStart( &view_x, &view_y );
    view_x *= wxPLOT_SCROLL_STEP;
    view_y *= wxPLOT_SCROLL_STEP;

    wxPaintDC dc( this );

    int client_width;
    int client_height;
    GetClientSize( &client_width, &client_height);

    double zoom = m_owner->GetZoom();

    double ups = m_owner->GetUnitsPerValue() / zoom;

    double start = view_x * ups;
    double end = (view_x + client_width) * ups;
    double range = end - start;

    int int_log_range = (int)floor( log10( range ) );
    double step = 1.0;
    if (int_log_range > 0)
    {
        for (int i = 0; i < int_log_range; i++)
           step *= 10;
    }
    if (int_log_range < 0)
    {
        for (int i = 0; i < -int_log_range; i++)
           step /= 10;
    }
    double lower = ceil(start / step) * step;
    double upper = floor(end / step) * step;

    // if too few values, shrink size
    if ((range/step) < 4)
    {
        step /= 2;
        if (lower-step > start) lower -= step;
        if (upper+step < end) upper += step;
    }

    // if still too few, again
    if ((range/step) < 4)
    {
        step /= 2;
        if (lower-step > start) lower -= step;
        if (upper+step < end) upper += step;
    }

    dc.SetBrush( *wxWHITE_BRUSH );
    dc.SetPen( *wxTRANSPARENT_PEN );
    dc.DrawRectangle( 4, 5, client_width-14, 10 );
    dc.DrawRectangle( 0, 20, client_width, 20 );
    dc.SetPen( *wxBLACK_PEN );

    double current = lower;
    while (current < upper+(step/2))
    {
        int x = (int)ceil((current-start) / range * (double)client_width) - 1;
        if ((x > 4) && (x < client_width-25))
        {
            dc.DrawLine( x, 5, x, 15 );
            wxString label;
            if (range < 50)
            {
                label.Printf( _T("%f"), current );
                while (label.Last() == _T('0'))
                    label.RemoveLast();
                if ((label.Last() == _T('.')) || (label.Last() == _T(',')))
                    label.Append( _T('0') );
            }
            else
                label.Printf( _T("%d"), (int)floor(current) );
            dc.DrawText( label, x-4, 20 );
        }

        current += step;
    }

    dc.DrawLine( 0, 15, client_width-8, 15 );
    dc.DrawLine( client_width-4, 15, client_width-10, 10 );
    dc.DrawLine( client_width-4, 15, client_width-10, 20 );
}

//-----------------------------------------------------------------------------
// wxPlotYAxisArea
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxPlotYAxisArea, wxWindow)

BEGIN_EVENT_TABLE(wxPlotYAxisArea, wxWindow)
  EVT_PAINT(        wxPlotYAxisArea::OnPaint)
  EVT_LEFT_DOWN(    wxPlotYAxisArea::OnMouse)
END_EVENT_TABLE()

wxPlotYAxisArea::wxPlotYAxisArea( wxPlotWindow *parent )
        : wxWindow( parent, wxID_ANY, wxDefaultPosition, wxSize(60,wxDefaultCoord), 0, _T("plotyaxisarea") )
{
    m_owner = parent;

    SetBackgroundColour( *wxWHITE );
    SetFont( *wxSMALL_FONT );
}

void wxPlotYAxisArea::OnMouse( wxMouseEvent &WXUNUSED(event) )
{
    /* do something here */
}

void wxPlotYAxisArea::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    wxPaintDC dc( this );

    wxPlotCurve *curve = m_owner->GetCurrentCurve();

    if (!curve) return;

    int client_width;
    int client_height;
    GetClientSize( &client_width, &client_height);


    double range = curve->GetEndY() - curve->GetStartY();
    double offset = ((double) curve->GetOffsetY() / (double)client_height ) * range;
    double start = curve->GetStartY() - offset;
    double end = curve->GetEndY() - offset;

    int int_log_range = (int)floor( log10( range ) );
    double step = 1.0;
    if (int_log_range > 0)
    {
        for (int i = 0; i < int_log_range; i++)
           step *= 10;
    }
    if (int_log_range < 0)
    {
        for (int i = 0; i < -int_log_range; i++)
           step /= 10;
    }
    double lower = ceil(start / step) * step;
    double upper = floor(end / step) * step;

    // if too few values, shrink size
    if ((range/step) < 4)
    {
        step /= 2;
        if (lower-step > start) lower -= step;
        if (upper+step < end) upper += step;
    }

    // if still too few, again
    if ((range/step) < 4)
    {
        step /= 2;
        if (lower-step > start) lower -= step;
        if (upper+step < end) upper += step;
    }

    dc.SetPen( *wxBLACK_PEN );

    double current = lower;
    while (current < upper+(step/2))
    {
        int y = (int)((curve->GetEndY()-current) / range * (double)client_height) - 1;
        y -= curve->GetOffsetY();
        if ((y > 10) && (y < client_height-7))
        {
            dc.DrawLine( client_width-15, y, client_width-7, y );
            wxString label;
            if (range < 50)
            {
                label.Printf( _T("%f"), current );
                while (label.Last() == _T('0'))
                    label.RemoveLast();
                if ((label.Last() == _T('.')) || (label.Last() == _T(',')))
                    label.Append( _T('0') );
            }
            else
                label.Printf( _T("%d"), (int)floor(current) );
            dc.DrawText( label, 5, y-7 );
        }

        current += step;
    }

    dc.DrawLine( client_width-15, 6, client_width-15, client_height );
    dc.DrawLine( client_width-15, 2, client_width-20, 8 );
    dc.DrawLine( client_width-15, 2, client_width-10, 8 );
}

//-----------------------------------------------------------------------------
// wxPlotWindow
//-----------------------------------------------------------------------------

#define  ID_ENLARGE       1000
#define  ID_SHRINK        1002

#define  ID_MOVE_UP       1006
#define  ID_MOVE_DOWN     1007

#define  ID_ZOOM_IN       1010
#define  ID_ZOOM_OUT      1011


IMPLEMENT_DYNAMIC_CLASS(wxPlotWindow, wxScrolledWindow)

BEGIN_EVENT_TABLE(wxPlotWindow, wxScrolledWindow)
    EVT_BUTTON(  ID_MOVE_UP,     wxPlotWindow::OnMoveUp)
    EVT_BUTTON(  ID_MOVE_DOWN,   wxPlotWindow::OnMoveDown)

    EVT_BUTTON(  ID_ENLARGE,  wxPlotWindow::OnEnlarge)
    EVT_BUTTON(  ID_SHRINK,   wxPlotWindow::OnShrink)

    EVT_BUTTON(  ID_ZOOM_IN,     wxPlotWindow::OnZoomIn)
    EVT_BUTTON(  ID_ZOOM_OUT,    wxPlotWindow::OnZoomOut)

    EVT_SCROLLWIN( wxPlotWindow::OnScroll2)
END_EVENT_TABLE()

wxPlotWindow::wxPlotWindow( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, int flag )
        : wxScrolledWindow( parent, id, pos, size, flag, _T("plotcanvas") ),
          m_titleStaticText( NULL )
{
    m_xUnitsPerValue = 1.0;
    m_xZoom = 1.0;

    m_enlargeAroundWindowCentre = false;
    m_scrollOnThumbRelease = false;

    m_area = new wxPlotArea( this );
    wxBoxSizer *mainsizer = new wxBoxSizer( wxHORIZONTAL );

    if ((GetWindowStyleFlag() & wxPLOT_BUTTON_ALL) != 0)
    {
        wxBoxSizer *buttonlist = new wxBoxSizer( wxVERTICAL );
        if ((GetWindowStyleFlag() & wxPLOT_BUTTON_ENLARGE) != 0)
        {
            buttonlist->Add( new wxBitmapButton( this, ID_ENLARGE, *GetEnlargeBitmap() ), 0, wxEXPAND|wxALL, 2 );
            buttonlist->Add( new wxBitmapButton( this, ID_SHRINK, *GetShrinkBitmap() ), 0, wxEXPAND|wxALL, 2 );
            buttonlist->Add( 20,10, 0 );
        }
        if ((GetWindowStyleFlag() & wxPLOT_BUTTON_MOVE) != 0)
        {
            buttonlist->Add( new wxBitmapButton( this, ID_MOVE_UP, *GetUpBitmap() ), 0, wxEXPAND|wxALL, 2 );
            buttonlist->Add( new wxBitmapButton( this, ID_MOVE_DOWN, *GetDownBitmap() ), 0, wxEXPAND|wxALL, 2 );
            buttonlist->Add( 20,10, 0 );
        }
        if ((GetWindowStyleFlag() & wxPLOT_BUTTON_ZOOM) != 0)
        {
            buttonlist->Add( new wxBitmapButton( this, ID_ZOOM_IN, *GetZoomInBitmap() ), 0, wxEXPAND|wxALL, 2 );
            buttonlist->Add( new wxBitmapButton( this, ID_ZOOM_OUT, *GetZoomOutBitmap() ), 0, wxEXPAND|wxALL, 2 );
        }
        mainsizer->Add( buttonlist, 0, wxEXPAND|wxALL, 4 );
    }

    wxBoxSizer *plotsizer = new wxBoxSizer( wxHORIZONTAL );

    //Add sizer to hold the title and plot.
    //Title to be added later.
    m_plotAndTitleSizer = new wxBoxSizer( wxVERTICAL );
    m_plotAndTitleSizer->Add( plotsizer, 1, wxEXPAND | wxTOP, 10 );

    if ((GetWindowStyleFlag() & wxPLOT_Y_AXIS) != 0)
    {
        m_yaxis = new wxPlotYAxisArea( this );

        wxBoxSizer *vert1 = new wxBoxSizer( wxVERTICAL );
        plotsizer->Add( vert1, 1, wxEXPAND|wxTOP,10 );
        vert1->Add( m_yaxis, 1 );
        if ((GetWindowStyleFlag() & wxPLOT_X_AXIS) != 0)
            vert1->Add( 60, 40 );
    }
    else
    {
        m_yaxis = (wxPlotYAxisArea*) NULL;
    }

    if ((GetWindowStyleFlag() & wxPLOT_X_AXIS) != 0)
    {
        m_xaxis = new wxPlotXAxisArea( this );

        wxBoxSizer *vert2 = new wxBoxSizer( wxVERTICAL );
        plotsizer->Add( vert2, 5, wxEXPAND);
        vert2->Add( m_area, 1, wxEXPAND|wxTOP,10 );
        vert2->Add( m_xaxis, 0, wxEXPAND );
    }
    else
    {
        plotsizer->Add( m_area, 1, wxEXPAND );
        m_xaxis = (wxPlotXAxisArea*) NULL;
    }

    mainsizer->Add( m_plotAndTitleSizer, 1, wxEXPAND );

    SetSizerAndFit( mainsizer );

    SetTargetWindow( m_area );

    SetBackgroundColour( *wxWHITE );

    m_current = (wxPlotCurve*) NULL;
}

wxPlotWindow::~wxPlotWindow()
{
}

void wxPlotWindow::Add( wxPlotCurve *curve )
{
    m_curves.Append( curve );
    if (!m_current) m_current = curve;

    ResetScrollbar();
}

size_t wxPlotWindow::GetCount()
{
    return m_curves.GetCount();
}

wxPlotCurve *wxPlotWindow::GetAt( size_t n )
{
    wxList::compatibility_iterator node = m_curves.Item( n );
    if (!node)
        return (wxPlotCurve*) NULL;

    return (wxPlotCurve*) node->GetData();
}

void wxPlotWindow::SetCurrentCurve( wxPlotCurve* current )
{
    m_current = current;
    m_area->Refresh( false );

    RedrawYAxis();

    wxPlotEvent event( wxEVT_PLOT_SEL_CHANGED, GetId() );
    event.SetEventObject( this );
    event.SetZoom( GetZoom() );
    event.SetCurve( m_current );
    GetEventHandler()->ProcessEvent( event );
}

void wxPlotWindow::Delete( wxPlotCurve* curve )
{
    wxList::compatibility_iterator node = m_curves.Find( curve );
    if (!node) return;

    m_curves.DeleteObject( curve );

    m_area->DeleteCurve( curve );
    m_area->Refresh( false );

    if (curve == m_current) m_current = (wxPlotCurve *) NULL;
}

wxPlotCurve *wxPlotWindow::GetCurrentCurve()
{
    return m_current;
}

void wxPlotWindow::Add( wxPlotOnOffCurve *curve )
{
    m_onOffCurves.Append( curve );
}

void wxPlotWindow::Delete( wxPlotOnOffCurve* curve )
{
    wxList::compatibility_iterator node = m_onOffCurves.Find( curve );
    if (!node) return;

    m_onOffCurves.DeleteObject( curve );
}

size_t wxPlotWindow::GetOnOffCurveCount()
{
    return m_onOffCurves.GetCount();
}

wxPlotOnOffCurve *wxPlotWindow::GetOnOffCurveAt( size_t n )
{
    wxList::compatibility_iterator node = m_onOffCurves.Item( n );
    if (!node)
        return (wxPlotOnOffCurve*) NULL;

    return (wxPlotOnOffCurve*) node->GetData();
}

void wxPlotWindow::Move( wxPlotCurve* curve, int pixels_up )
{
    m_area->DeleteCurve( curve );

    curve->SetOffsetY( curve->GetOffsetY() + pixels_up );

    m_area->Refresh( false );

    RedrawYAxis();
}

void wxPlotWindow::OnMoveUp( wxCommandEvent& WXUNUSED(event) )
{
    if (!m_current) return;

    Move( m_current, 25 );
}

void wxPlotWindow::OnMoveDown( wxCommandEvent& WXUNUSED(event) )
{
    if (!m_current) return;

    Move( m_current, -25 );
}

void wxPlotWindow::Enlarge( wxPlotCurve *curve, double factor )
{
    m_area->DeleteCurve( curve );

    int client_width;
    int client_height;
    m_area->GetClientSize( &client_width, &client_height);
    double offset = (double)curve->GetOffsetY() / (double)client_height;

    double range = curve->GetEndY() - curve->GetStartY();
    offset *= range;

    double new_range = range / factor;
    double new_offset = offset / factor;

    if (m_enlargeAroundWindowCentre)
    {
        double middle = curve->GetStartY() - offset + range/2;

        curve->SetStartY( middle - new_range / 2 + new_offset );
        curve->SetEndY( middle + new_range / 2 + new_offset  );
    }
    else
    {
        curve->SetStartY( (curve->GetStartY() - offset)/factor + new_offset );
        curve->SetEndY( (curve->GetEndY() - offset)/factor + new_offset );
    }

    m_area->Refresh( false );
    RedrawYAxis();
}

void wxPlotWindow::SetUnitsPerValue( double upv )
{
    m_xUnitsPerValue = upv;

    RedrawXAxis();
}

void wxPlotWindow::SetZoom( double zoom )
{
    double old_zoom = m_xZoom;
    m_xZoom = zoom;

    int view_x = 0;
    int view_y = 0;
    GetViewStart( &view_x, &view_y );

    wxInt32 max = 0;
    wxList::compatibility_iterator node = m_curves.GetFirst();
    while (node)
    {
        wxPlotCurve *curve = (wxPlotCurve*) node->GetData();
        if (curve->GetEndX() > max)
            max = curve->GetEndX();
        node = node->GetNext();
    }
    SetScrollbars( wxPLOT_SCROLL_STEP, wxPLOT_SCROLL_STEP,
                   (int)((max*m_xZoom)/wxPLOT_SCROLL_STEP)+1, 0,
                   (int)(view_x*zoom/old_zoom), 0,
                   true );

    RedrawXAxis();
    m_area->Refresh( true );
}

void wxPlotWindow::ResetScrollbar()
{
    wxInt32 max = 0;
    wxList::compatibility_iterator node = m_curves.GetFirst();
    while (node)
    {
        wxPlotCurve *curve = (wxPlotCurve*) node->GetData();
        if (curve->GetEndX() > max)
            max = curve->GetEndX();
        node = node->GetNext();
    }

    SetScrollbars( wxPLOT_SCROLL_STEP, wxPLOT_SCROLL_STEP,
                   (int)(((max*m_xZoom)/wxPLOT_SCROLL_STEP)+1), 0 );
}

void wxPlotWindow::AddChartTitle(const wxString& title, wxFont font,
                                 wxColour colour)
{
    m_title = title;
    m_titleFont = font;
    m_titleColour = colour;
    DrawChartTitle();
}

void wxPlotWindow::DrawChartTitle()
{
    if(m_title.size() != 0)
    {
        //If it is already added, remove child and delete
        if(m_titleStaticText)
        {
            RemoveChild( m_titleStaticText );
            m_titleStaticText->Destroy();
        }

        //Create the text control and set the font, colour
        m_titleStaticText = new wxStaticText( this, -1, m_title );
        m_titleStaticText->SetFont( m_titleFont );
        m_titleStaticText->SetForegroundColour( m_titleColour );

        //Create a sizer for the title. Prepend it to the Plot + Title sizer.
        wxBoxSizer* titleSizer = new wxBoxSizer( wxHORIZONTAL );
        titleSizer->Add( m_titleStaticText, 0, wxALIGN_CENTER | wxALL, 10 );
        m_plotAndTitleSizer->Prepend( titleSizer, 0, wxALIGN_CENTER_HORIZONTAL );

        //Finally, force layout
        m_plotAndTitleSizer->Layout();
    }
}

void wxPlotWindow::RedrawXAxis()
{
    if (m_xaxis)
        m_xaxis->Refresh( true );
}

void wxPlotWindow::RedrawYAxis()
{
    if (m_yaxis)
       m_yaxis->Refresh( true );
}

void wxPlotWindow::RedrawEverything()
{
    if (m_xaxis)
        m_xaxis->Refresh( true );
    if (m_yaxis)
        m_yaxis->Refresh( true );
    m_area->Refresh( true );

    DrawChartTitle();
}

void wxPlotWindow::OnZoomIn( wxCommandEvent& WXUNUSED(event) )
{
    SetZoom( m_xZoom * 1.5 );
}

void wxPlotWindow::OnZoomOut( wxCommandEvent& WXUNUSED(event) )
{
    SetZoom( m_xZoom * 0.6666 );
}

void wxPlotWindow::OnEnlarge( wxCommandEvent& WXUNUSED(event) )
{
    if (!m_current) return;

    Enlarge( m_current, 1.5 );
}

void wxPlotWindow::OnShrink( wxCommandEvent& WXUNUSED(event) )
{
    if (!m_current) return;

    Enlarge( m_current, 0.6666666 );
}

void wxPlotWindow::OnScroll2( wxScrollWinEvent& event )
{
    if ((!m_scrollOnThumbRelease) || (event.GetEventType() != wxEVT_SCROLLWIN_THUMBTRACK))
    {
        wxScrolledWindow::OnScroll( event );
        RedrawXAxis();
    }
}

// ----------------------------------------------------------------------------
// global functions
// ----------------------------------------------------------------------------

// FIXME MT-UNSAFE
static wxBitmap *GetEnlargeBitmap()
{
    static wxBitmap* s_bitmap = (wxBitmap *) NULL;
    static bool s_loaded = false;

    if ( !s_loaded )
    {
        s_loaded = true; // set it to true anyhow, we won't try again

        #if defined(__WXMSW__) || defined(__WXPM__)
            s_bitmap = new wxBitmap(_T("plot_enl_bmp"), wxBITMAP_TYPE_RESOURCE);
        #else
            s_bitmap = new wxBitmap( plot_enl_xpm );
        #endif
    }

    return s_bitmap;
}

static wxBitmap *GetShrinkBitmap()
{
    static wxBitmap* s_bitmap = (wxBitmap *) NULL;
    static bool s_loaded = false;

    if ( !s_loaded )
    {
        s_loaded = true; // set it to true anyhow, we won't try again

        #if defined(__WXMSW__) || defined(__WXPM__)
            s_bitmap = new wxBitmap(_T("plot_shr_bmp"), wxBITMAP_TYPE_RESOURCE);
        #else
            s_bitmap = new wxBitmap( plot_shr_xpm );
        #endif
    }

    return s_bitmap;
}

static wxBitmap *GetZoomInBitmap()
{
    static wxBitmap* s_bitmap = (wxBitmap *) NULL;
    static bool s_loaded = false;

    if ( !s_loaded )
    {
        s_loaded = true; // set it to true anyhow, we won't try again

        #if defined(__WXMSW__) || defined(__WXPM__)
            s_bitmap = new wxBitmap(_T("plot_zin_bmp"), wxBITMAP_TYPE_RESOURCE);
        #else
            s_bitmap = new wxBitmap( plot_zin_xpm );
        #endif
    }

    return s_bitmap;
}

static wxBitmap *GetZoomOutBitmap()
{
    static wxBitmap* s_bitmap = (wxBitmap *) NULL;
    static bool s_loaded = false;

    if ( !s_loaded )
    {
        s_loaded = true; // set it to true anyhow, we won't try again

        #if defined(__WXMSW__) || defined(__WXPM__)
            s_bitmap = new wxBitmap(_T("plot_zot_bmp"), wxBITMAP_TYPE_RESOURCE);
        #else
            s_bitmap = new wxBitmap( plot_zot_xpm );
        #endif
    }

    return s_bitmap;
}

static wxBitmap *GetUpBitmap()
{
    static wxBitmap* s_bitmap = (wxBitmap *) NULL;
    static bool s_loaded = false;

    if ( !s_loaded )
    {
        s_loaded = true; // set it to true anyhow, we won't try again

        #if defined(__WXMSW__) || defined(__WXPM__)
            s_bitmap = new wxBitmap(_T("plot_up_bmp"), wxBITMAP_TYPE_RESOURCE);
        #else
            s_bitmap = new wxBitmap( plot_up_xpm );
        #endif
    }

    return s_bitmap;
}

static wxBitmap *GetDownBitmap()
{
    static wxBitmap* s_bitmap = (wxBitmap *) NULL;
    static bool s_loaded = false;

    if ( !s_loaded )
    {
        s_loaded = true; // set it to true anyhow, we won't try again

        #if defined(__WXMSW__) || defined(__WXPM__)
            s_bitmap = new wxBitmap(_T("plot_dwn_bmp"), wxBITMAP_TYPE_RESOURCE);
        #else
            s_bitmap = new wxBitmap( plot_dwn_xpm );
        #endif
    }

    return s_bitmap;
}

