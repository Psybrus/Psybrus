/////////////////////////////////////////////////////////////////////////////
// Name:        wx/cocoa/bitmap.h
// Purpose:     wxBitmap class
// Author:      David Elliott
// Modified by:
// Created:     2003/07/19
// RCS-ID:      $Id: bitmap.h 47992 2007-08-09 19:08:30Z DE $
// Copyright:   (c) 2003 David Elliott
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_COCOA_BITMAP_H__
#define __WX_COCOA_BITMAP_H__

#include "wx/palette.h"

// Bitmap
class WXDLLEXPORT wxBitmap;
class WXDLLEXPORT wxIcon;
class WXDLLEXPORT wxCursor;
class WXDLLEXPORT wxImage;
class WXDLLEXPORT wxPixelDataBase;

// ========================================================================
// wxMask
// ========================================================================

// A mask is a 1-bit alpha bitmap used for drawing bitmaps transparently.
class WXDLLEXPORT wxMask: public wxObject
{
    DECLARE_DYNAMIC_CLASS(wxMask)
public:
    wxMask();

    // Construct a mask from a bitmap and a colour indicating
    // the transparent area
    wxMask(const wxBitmap& bitmap, const wxColour& colour);

    // Construct a mask from a bitmap and a palette index indicating
    // the transparent area
    wxMask(const wxBitmap& bitmap, int paletteIndex);

    // Construct a mask from a mono bitmap (copies the bitmap).
    wxMask(const wxBitmap& bitmap);

    // Copy constructor
    wxMask(const wxMask& src);

    virtual ~wxMask();

    bool Create(const wxBitmap& bitmap, const wxColour& colour);
    bool Create(const wxBitmap& bitmap, int paletteIndex);
    bool Create(const wxBitmap& bitmap);

    // wxCocoa
    inline WX_NSBitmapImageRep GetNSBitmapImageRep()
    {   return m_cocoaNSBitmapImageRep; }
protected:
    WX_NSBitmapImageRep m_cocoaNSBitmapImageRep;
};

// ========================================================================
// wxBitmap
// ========================================================================
class WXDLLEXPORT wxBitmap: public wxGDIObject
{
    DECLARE_DYNAMIC_CLASS(wxBitmap)
// ------------------------------------------------------------------------
// initialization
// ------------------------------------------------------------------------
public:
    // Platform-specific default constructor
    wxBitmap();
    // Initialize with raw data.
    wxBitmap(const char bits[], int width, int height, int depth = 1);
    // Initialize with XPM data
    wxBitmap(const char* const* bits);
    // Load a file or resource
    wxBitmap(const wxString& name, wxBitmapType type = wxBITMAP_TYPE_BMP_RESOURCE);
    // Construct from Cocoa's NSImage
    wxBitmap(NSImage* cocoaNSImage);
    // Construct from Cocoa's NSBitmapImageRep
    wxBitmap(NSBitmapImageRep* cocoaNSBitmapImageRep);
    // Constructor for generalised creation from data
    wxBitmap(const void* data, wxBitmapType type, int width, int height, int depth = 1);
    // If depth is omitted, will create a bitmap compatible with the display
    wxBitmap(int width, int height, int depth = -1);
    // Convert from wxImage:
    wxBitmap(const wxImage& image, int depth = -1)
    {   CreateFromImage(image, depth); }
    // Convert from wxIcon
    wxBitmap(const wxIcon& icon) { CopyFromIcon(icon); }

    // destructor
    virtual ~wxBitmap();

// ------------------------------------------------------------------------
// Implementation
// ------------------------------------------------------------------------
public:
    // Initialize from wxImage
    bool CreateFromImage(const wxImage& image, int depth=-1);

    virtual bool Create(int width, int height, int depth = -1);
    bool Create(NSImage* cocoaNSImage);
    bool Create(NSBitmapImageRep* cocoaNSBitmapImageRep);
    virtual bool Create(const void* data, wxBitmapType type, int width, int height, int depth = 1);
    virtual bool LoadFile(const wxString& name, wxBitmapType type = wxBITMAP_TYPE_BMP_RESOURCE);
    virtual bool SaveFile(const wxString& name, wxBitmapType type, const wxPalette *cmap = NULL) const;

    // copies the contents and mask of the given (colour) icon to the bitmap
    virtual bool CopyFromIcon(const wxIcon& icon);

    wxImage ConvertToImage() const;

    // get the given part of bitmap
    wxBitmap GetSubBitmap( const wxRect& rect ) const;

    bool Ok() const { return IsOk(); }
    bool IsOk() const;
    int GetWidth() const;
    int GetHeight() const;
    int GetDepth() const;
    int GetQuality() const;
    void SetWidth(int w);
    void SetHeight(int h);
    void SetDepth(int d);
    void SetQuality(int q);
    void SetOk(bool isOk);

    // raw bitmap access support functions
    void *GetRawData(wxPixelDataBase& data, int bpp);
    void UngetRawData(wxPixelDataBase& data);
    void UseAlpha();

    wxPalette* GetPalette() const;
    void SetPalette(const wxPalette& palette);

    wxMask *GetMask() const;
    void SetMask(wxMask *mask) ;

    int GetBitmapType() const;

    // wxObjectRefData
    wxObjectRefData *CreateRefData() const;
    wxObjectRefData *CloneRefData(const wxObjectRefData *data) const;

    // wxCocoa
    WX_NSBitmapImageRep GetNSBitmapImageRep();
    void SetNSBitmapImageRep(WX_NSBitmapImageRep bitmapImageRep);
    WX_NSImage GetNSImage(bool useMask) const;

    static void InitStandardHandlers() { }
    static void CleanUpHandlers() { }
};

class WXDLLIMPEXP_CORE wxBitmapHandler: public wxBitmapHandlerBase
{
    DECLARE_ABSTRACT_CLASS(wxBitmapHandler)
};

#endif
    // __WX_COCOA_BITMAP_H__
