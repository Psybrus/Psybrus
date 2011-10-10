/////////////////////////////////////////////////////////////////////////////
// Name:        wx/cocoa/statbmp.h
// Purpose:     wxStaticBitmap class
// Author:      David Elliott
// Modified by:
// Created:     2003/03/16
// RCS-ID:      $Id: statbmp.h 48114 2007-08-15 17:58:46Z DE $
// Copyright:   (c) 2003 David Elliott
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_COCOA_STATBMP_H__
#define __WX_COCOA_STATBMP_H__

DECLARE_WXCOCOA_OBJC_CLASS(NSImageView);

// ========================================================================
// wxStaticBitmap
// ========================================================================
class WXDLLEXPORT wxStaticBitmap : public wxStaticBitmapBase //, protected wxCocoaNSxxx
{
    DECLARE_DYNAMIC_CLASS(wxStaticBitmap)
    DECLARE_EVENT_TABLE()
//    WX_DECLARE_COCOA_OWNER(NSxxx,NSControl,NSView)
// ------------------------------------------------------------------------
// initialization
// ------------------------------------------------------------------------
public:
    wxStaticBitmap() {}
    wxStaticBitmap(wxWindow *parent, wxWindowID winid,
            const wxBitmap& bitmap,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0,
            const wxString& name = wxStaticBitmapNameStr)
    {
        Create(parent, winid, bitmap, pos, size, style, name);
    }

    bool Create(wxWindow *parent, wxWindowID winid,
            const wxBitmap& bitmap,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0,
            const wxString& name = wxStaticBitmapNameStr);
    virtual ~wxStaticBitmap();

// ------------------------------------------------------------------------
// Cocoa specifics
// ------------------------------------------------------------------------
    WX_NSImageView GetNSImageView() { return (WX_NSImageView)m_cocoaNSView; }
#if wxUSE_ABI_INCOMPATIBLE_FEATURES
    wxBitmap m_bitmap;
#endif

// ------------------------------------------------------------------------
// Implementation
// ------------------------------------------------------------------------
    virtual void SetIcon(const wxIcon& icon);
    virtual void SetBitmap(const wxBitmap& bitmap);
    virtual wxBitmap GetBitmap() const;
};

#endif // __WX_COCOA_STATBMP_H__
