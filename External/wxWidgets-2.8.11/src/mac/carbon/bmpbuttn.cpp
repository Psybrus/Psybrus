/////////////////////////////////////////////////////////////////////////////
// Name:        src/mac/carbon/bmpbuttn.cpp
// Purpose:     wxBitmapButton
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: bmpbuttn.cpp 53339 2008-04-24 23:19:36Z VS $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#if wxUSE_BMPBUTTON

#include "wx/bmpbuttn.h"
#include "wx/image.h"

#ifndef WX_PRECOMP
    #include "wx/dcmemory.h"
#endif

IMPLEMENT_DYNAMIC_CLASS(wxBitmapButton, wxButton)

#include "wx/mac/uma.h"

//---------------------------------------------------------------------------
// Helper functions

static wxBitmap wxMakeStdSizeBitmap(const wxBitmap& bitmap)
{
    // in Mac OS X the icon controls (which are used for borderless bitmap
    // buttons) can have only one of the few standard sizes and if they
    // don't, the OS rescales them automatically resulting in really ugly
    // images, so centre the image in a square of standard size instead

    // the supported sizes, sorted in decreasng order
    static const int stdSizes[] = { 128, 48, 32, 16, 0 };

    const int width = bitmap.GetWidth();
    const int height = bitmap.GetHeight();

    wxBitmap newBmp(bitmap);
    
    int n;
    for ( n = 0; n < (int)WXSIZEOF(stdSizes); n++ )
    {
        const int sizeStd = stdSizes[n];
        if ( width > sizeStd || height > sizeStd )
        {
            // it will become -1 if the bitmap is larger than the biggest
            // supported size, this is intentional
            n--;

            break;
        }
    }

    if ( n != -1 )
    {
        const int sizeStd = stdSizes[n];
        if ( width != sizeStd || height != sizeStd )
        {
            wxASSERT_MSG( width <= sizeStd && height <= sizeStd,
                          _T("bitmap shouldn't be cropped") );

            wxImage square_image = bitmap.ConvertToImage();
            newBmp = square_image.Size
                     (
                         wxSize(sizeStd, sizeStd),
                         wxPoint((sizeStd - width)/2, (sizeStd-height)/2)
                     );
        }
    }
    //else: let the system rescale the bitmap

    return newBmp;
}

//---------------------------------------------------------------------------

bool wxBitmapButton::Create( wxWindow *parent,
                             wxWindowID id, const wxBitmap& bitmap,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxValidator& validator,
                             const wxString& name )
{
    m_macIsUserPane = false;

    // since bitmapbuttonbase is subclass of button calling wxBitmapButtonBase::Create
    // essentially creates an additional button
    if ( !wxControl::Create( parent, id, pos, size, style, validator, name ) )
        return false;

    if ( style & wxBU_AUTODRAW )
    {
        m_marginX =
        m_marginY = wxDEFAULT_BUTTON_MARGIN;
    }
    else
    {
        m_marginX =
        m_marginY = 0;
    }

    OSStatus err = noErr;
    ControlButtonContentInfo info;

    Rect bounds = wxMacGetBoundsForControl( this, pos, size );
    m_peer = new wxMacControl( this );

    if ( bitmap.Ok() && HasFlag(wxBORDER_NONE) )
        m_bmpNormal = wxMakeStdSizeBitmap(bitmap);
    else
        m_bmpNormal = bitmap;


#ifdef __WXMAC_OSX__
    if ( HasFlag( wxBORDER_NONE ) )
    {
		// contrary to the docs this control only works with iconrefs
		wxMacCreateBitmapButton( &info, m_bmpNormal, kControlContentIconRef );
        err = CreateIconControl(
                MAC_WXHWND(parent->MacGetTopLevelWindowRef()),
                &bounds, &info, false, m_peer->GetControlRefAddr() );
    }
    else
#endif
    {
        wxMacCreateBitmapButton( &info, m_bmpNormal );
        err = CreateBevelButtonControl(
                MAC_WXHWND(parent->MacGetTopLevelWindowRef()), &bounds, CFSTR(""),
                ((style & wxBU_AUTODRAW) ? kControlBevelButtonSmallBevel : kControlBevelButtonNormalBevel ),
                kControlBehaviorOffsetContents, &info, 0, 0, 0, m_peer->GetControlRefAddr() );
    }

    verify_noerr( err );

    wxMacReleaseBitmapButton( &info );
    wxASSERT_MSG( m_peer != NULL && m_peer->Ok(), wxT("No valid native Mac control") );

    MacPostControlCreate( pos, size );

    return true;
}

void wxBitmapButton::SetBitmapLabel( const wxBitmap& bitmap )
{
    if ( HasFlag( wxBORDER_NONE ) )
        m_bmpNormal = wxMakeStdSizeBitmap(bitmap);
    else
        m_bmpNormal = bitmap;
    
    InvalidateBestSize();

    ControlButtonContentInfo info;

#ifdef __WXMAC_OSX__
    if ( HasFlag( wxBORDER_NONE ) )
    {        
        wxMacCreateBitmapButton( &info, m_bmpNormal, kControlContentIconRef );
        if ( info.contentType != kControlNoContent )
            m_peer->SetData( kControlIconPart, kControlIconContentTag, info );
    }
    else
#endif
    {
        wxMacCreateBitmapButton( &info, m_bmpNormal );
        if ( info.contentType != kControlNoContent )
            m_peer->SetData( kControlButtonPart, kControlBevelButtonContentTag, info );
    }

    wxMacReleaseBitmapButton( &info );
}

wxSize wxBitmapButton::DoGetBestSize() const
{
    wxSize best;

    best.x = 2 * m_marginX;
    best.y = 2 * m_marginY;
    if ( m_bmpNormal.Ok() )
    {
        best.x += m_bmpNormal.GetWidth();
        best.y += m_bmpNormal.GetHeight();
    }

    return best;
}

#endif
