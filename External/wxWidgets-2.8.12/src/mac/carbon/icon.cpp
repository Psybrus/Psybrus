/////////////////////////////////////////////////////////////////////////////
// Name:        src/mac/carbon/icon.cpp
// Purpose:     wxIcon class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: icon.cpp 49034 2007-10-04 07:32:09Z SC $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "wx/icon.h"

#ifndef WX_PRECOMP
    #include "wx/image.h"
#endif

#include "wx/mac/private.h"

IMPLEMENT_DYNAMIC_CLASS(wxIcon, wxBitmap)

#define M_ICONDATA ((wxIconRefData *)m_refData)


wxIcon::wxIcon()
{
}

wxIcon::wxIcon( const char bits[], int width, int height )
{
    wxBitmap bmp( bits, width, height ) ;
    CopyFromBitmap( bmp ) ;
}

wxIcon::wxIcon( const char **bits )
{
    wxBitmap bmp( bits ) ;
    CopyFromBitmap( bmp ) ;
}

wxIcon::wxIcon( char **bits )
{
    wxBitmap bmp( bits ) ;
    CopyFromBitmap( bmp ) ;
}

wxIcon::wxIcon(
    const wxString& icon_file, int flags,
    int desiredWidth, int desiredHeight )
{
    LoadFile( icon_file, (wxBitmapType) flags, desiredWidth, desiredHeight );
}

wxIcon::~wxIcon()
{
}

WXHICON wxIcon::GetHICON() const
{
    wxASSERT( Ok() ) ;

    return (WXHICON) ((wxIconRefData*)m_refData)->GetHICON() ;
}

int wxIcon::GetWidth() const
{
   wxCHECK_MSG( Ok(), -1, wxT("invalid icon") );

   return M_ICONDATA->GetWidth();
}

int wxIcon::GetHeight() const
{
   wxCHECK_MSG( Ok(), -1, wxT("invalid icon") );

   return M_ICONDATA->GetHeight();
}

int wxIcon::GetDepth() const
{
    return 32;
}

void wxIcon::SetDepth( int depth )
{
}

void wxIcon::SetWidth( int width )
{
}

void wxIcon::SetHeight( int height )
{
}

bool wxIcon::IsOk() const
{
    return m_refData != NULL ;
}

bool wxIcon::LoadFile(
    const wxString& filename, wxBitmapType type,
    int desiredWidth, int desiredHeight )
{
    UnRef();

    if ( type == wxBITMAP_TYPE_ICON_RESOURCE )
    {
        OSType theId = 0 ;

        if ( filename == wxT("wxICON_INFORMATION") )
        {
            theId = kAlertNoteIcon ;
        }
        else if ( filename == wxT("wxICON_QUESTION") )
        {
            theId = kAlertCautionIcon ;
        }
        else if ( filename == wxT("wxICON_WARNING") )
        {
            theId = kAlertCautionIcon ;
        }
        else if ( filename == wxT("wxICON_ERROR") )
        {
            theId = kAlertStopIcon ;
        }
        else
        {
        	IconRef iconRef = NULL ;
        	
        	// first look in the resource fork
        	if ( iconRef == NULL )
        	{
	        	Str255 theName ;

	        	wxMacStringToPascal( filename , theName ) ;
	        	Handle resHandle = GetNamedResource( 'icns' , theName ) ;
	        	if ( resHandle != 0L )
	        	{
					IconFamilyHandle iconFamily = (IconFamilyHandle) resHandle ;
                    HLock((Handle) iconFamily);
                    OSStatus err = GetIconRefFromIconFamilyPtr( *iconFamily, GetHandleSize((Handle) iconFamily), &iconRef );
                    HUnlock((Handle) iconFamily);
                    wxASSERT_MSG( err == noErr , wxT("Error when constructing icon ref") );
                    ReleaseResource( resHandle ) ;
	        	}
  			}
            if ( iconRef == NULL )
            {
                // TODO add other attempts to load it from files etc here
            }
	   		if ( iconRef )
	   		{
               	m_refData = new wxIconRefData( (WXHICON) iconRef ) ;
	        	return true ;
	   		}      	
        }

        if ( theId != 0 )
        {
            IconRef iconRef = NULL ;
            verify_noerr( GetIconRef( kOnSystemDisk, kSystemIconsCreator, theId, &iconRef ) ) ;
            if ( iconRef )
            {
                m_refData = new wxIconRefData( (WXHICON) iconRef ) ;

                return true ;
            }
        }

        return false ;
    }
    else
    {
        wxBitmapHandler *handler = wxBitmap::FindHandler( type );

        if ( handler )
        {
            wxBitmap bmp ;
            if ( handler->LoadFile( &bmp , filename, type, desiredWidth, desiredHeight ))
            {
                CopyFromBitmap( bmp ) ;

                return true ;
            }

            return false ;
        }
        else
        {
#if wxUSE_IMAGE
            wxImage loadimage( filename, type );
            if (loadimage.Ok())
            {
                if ( desiredWidth == -1 )
                    desiredWidth = loadimage.GetWidth() ;
                if ( desiredHeight == -1 )
                    desiredHeight = loadimage.GetHeight() ;
                if ( desiredWidth != loadimage.GetWidth() || desiredHeight != loadimage.GetHeight() )
                    loadimage.Rescale( desiredWidth , desiredHeight ) ;

                wxBitmap bmp( loadimage );
                CopyFromBitmap( bmp ) ;

                return true;
            }
#endif
        }
    }
    return true ;
}

void wxIcon::CopyFromBitmap( const wxBitmap& bmp )
{
    UnRef() ;

    // as the bitmap owns that ref, we have to acquire it as well
    IconRef iconRef = bmp.GetBitmapData()->GetIconRef() ;
    AcquireIconRef( iconRef ) ;

    m_refData = new wxIconRefData( (WXHICON) iconRef ) ;
    M_ICONDATA->SetWidth( bmp.GetWidth() ) ;
    M_ICONDATA->SetHeight( bmp.GetHeight() ) ;
}

wxIconRefData::wxIconRefData( WXHICON icon )
{
    m_iconRef = MAC_WXHICON( icon ) ;

    // Standard sizes
    SetWidth( 32 ) ;
    SetHeight( 32 ) ;
}

void wxIconRefData::Init()
{
    m_iconRef = NULL ;
}

void wxIconRefData::Free()
{
    if ( m_iconRef )
    {
        ReleaseIconRef( m_iconRef ) ;
        m_iconRef = NULL ;
    }
}

IMPLEMENT_DYNAMIC_CLASS(wxICONResourceHandler, wxBitmapHandler)

bool  wxICONResourceHandler::LoadFile(
    wxBitmap *bitmap, const wxString& name, long flags,
    int desiredWidth, int desiredHeight )
{
    wxIcon icon ;
    icon.LoadFile( name , wxBITMAP_TYPE_ICON_RESOURCE , desiredWidth , desiredHeight ) ;
    bitmap->CopyFromIcon( icon ) ;

    return bitmap->Ok() ;
}

