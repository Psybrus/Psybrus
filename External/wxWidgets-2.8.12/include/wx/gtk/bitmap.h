/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/bitmap.h
// Purpose:
// Author:      Robert Roebling
// RCS-ID:      $Id: bitmap.h 42881 2006-11-01 01:16:01Z SN $
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_BITMAP_H_
#define _WX_GTK_BITMAP_H_

typedef struct _GdkPixbuf GdkPixbuf;
class WXDLLEXPORT wxPixelDataBase;

//-----------------------------------------------------------------------------
// wxMask
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxMask: public wxObject
{
public:
    wxMask();
    wxMask( const wxBitmap& bitmap, const wxColour& colour );
#if wxUSE_PALETTE
    wxMask( const wxBitmap& bitmap, int paletteIndex );
#endif // wxUSE_PALETTE
    wxMask( const wxBitmap& bitmap );
    virtual ~wxMask();

    bool Create( const wxBitmap& bitmap, const wxColour& colour );
#if wxUSE_PALETTE
    bool Create( const wxBitmap& bitmap, int paletteIndex );
#endif // wxUSE_PALETTE
    bool Create( const wxBitmap& bitmap );

    // implementation
    GdkBitmap   *m_bitmap;

    GdkBitmap *GetBitmap() const;

private:
    DECLARE_DYNAMIC_CLASS(wxMask)
};

//-----------------------------------------------------------------------------
// wxBitmap
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxBitmap: public wxBitmapBase
{
public:
    wxBitmap() { }
    wxBitmap( int width, int height, int depth = -1 );
    wxBitmap( const char bits[], int width, int height, int depth = 1 );
    wxBitmap( const char* const* bits );
#ifdef wxNEEDS_CHARPP
    // needed for old GCC
    wxBitmap(char** data)
    {
        *this = wxBitmap(wx_const_cast(const char* const*, data));
    }
#endif
    wxBitmap( const wxString &filename, wxBitmapType type = wxBITMAP_TYPE_XPM );
    wxBitmap( const wxImage& image, int depth = -1 ) { (void)CreateFromImage(image, depth); }
    virtual ~wxBitmap();
    bool Ok() const { return IsOk(); }
    bool IsOk() const;

    bool Create(int width, int height, int depth = -1);

    int GetHeight() const;
    int GetWidth() const;
    int GetDepth() const;

    wxImage ConvertToImage() const;

    // copies the contents and mask of the given (colour) icon to the bitmap
    virtual bool CopyFromIcon(const wxIcon& icon);

    wxMask *GetMask() const;
    void SetMask( wxMask *mask );

    wxBitmap GetSubBitmap( const wxRect& rect ) const;

    bool SaveFile(const wxString &name, wxBitmapType type,
                          const wxPalette *palette = (wxPalette *)NULL) const;
    bool LoadFile(const wxString &name, wxBitmapType type = wxBITMAP_TYPE_XPM );

#if wxUSE_PALETTE
    wxPalette *GetPalette() const;
    void SetPalette(const wxPalette& palette);
    wxPalette *GetColourMap() const { return GetPalette(); };
#endif // wxUSE_PALETTE

    static void InitStandardHandlers();

    // implementation
    // --------------

    void SetHeight( int height );
    void SetWidth( int width );
    void SetDepth( int depth );
    void SetPixmap( GdkPixmap *pixmap );
    void SetPixbuf(GdkPixbuf* pixbuf, int depth = 0);

    GdkPixmap *GetPixmap() const;
    bool HasPixmap() const;
    bool HasPixbuf() const;
    GdkPixbuf *GetPixbuf() const;

    // Basically, this corresponds to Win32 StretchBlt()
    wxBitmap Rescale(int clipx, int clipy, int clipwidth, int clipheight, int width, int height) const;

    // raw bitmap access support functions
    void *GetRawData(wxPixelDataBase& data, int bpp);
    void UngetRawData(wxPixelDataBase& data);

    bool HasAlpha() const;
    void UseAlpha();

protected:
    bool CreateFromImage(const wxImage& image, int depth);

    virtual wxObjectRefData* CreateRefData() const;
    virtual wxObjectRefData* CloneRefData(const wxObjectRefData* data) const;

private:
    // to be called from CreateFromImage only!
    bool CreateFromImageAsPixmap(const wxImage& image, int depth);
    bool CreateFromImageAsPixbuf(const wxImage& image);

    enum Representation
    {
        Pixmap,
        Pixbuf
    };
    // removes other representations from memory, keeping only 'keep'
    // (wxBitmap may keep same bitmap e.g. as both pixmap and pixbuf):
    void PurgeOtherRepresentations(Representation keep);

    friend class wxMemoryDC;
    friend class wxBitmapHandler;

private:
    DECLARE_DYNAMIC_CLASS(wxBitmap)
};

//-----------------------------------------------------------------------------
// wxBitmapHandler
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxBitmapHandler: public wxBitmapHandlerBase
{
    DECLARE_ABSTRACT_CLASS(wxBitmapHandler)
};

#endif // _WX_GTK_BITMAP_H_
