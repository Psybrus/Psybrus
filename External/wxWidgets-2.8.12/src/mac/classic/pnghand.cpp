/////////////////////////////////////////////////////////////////////////////
// Name:        pnghand.cpp
// Purpose:     Implements a PNG reader class + handler
// Author:      Julian Smart
// Modified by: 
// Created:     04/01/98
// RCS-ID:      $Id: pnghand.cpp 35650 2005-09-23 12:56:45Z MR $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#  pragma hdrstop
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if wxUSE_IOSTREAMH
#   include <fstream.h>
#else
#   include <fstream>
#endif

#ifndef __DARWIN__
#  include <windows.h>
#endif
#include "wx/msgdlg.h"
#include "wx/palette.h"
#include "wx/bitmap.h"
#include "wx/mac/pnghand.h"
#include "wx/mac/pngread.h"
#include "wx/mac/private.h"

extern "C" {
#include "png.h"
}

extern "C" void png_read_init PNGARG((png_structp png_ptr));
extern "C" void png_write_init PNGARG((png_structp png_ptr));

extern CTabHandle wxMacCreateColorTable( int numColors ) ;
extern void wxMacDestroyColorTable( CTabHandle colors )  ;
extern void wxMacSetColorTableEntry( CTabHandle newColors , int index , int red , int green ,  int blue ) ;
extern GWorldPtr wxMacCreateGWorld( int width , int height , int depth ) ;
extern void wxMacDestroyGWorld( GWorldPtr gw ) ;

void
ima_png_error(png_struct *png_ptr, char *message)
{
    wxMessageBox(wxString::FromAscii(message), wxT("PNG error"));
    longjmp(png_ptr->jmpbuf, 1);
}


// static wxGifReaderIter* iter;
wxPalette *wxCopyPalette(const wxPalette *cmap);

wxPNGReader::wxPNGReader(void)
{
    filetype = 0;
    RawImage = NULL;      //  Image data
    
    Width = 0; Height = 0;       //  Dimensions
    Depth = 0;           // (bits x pixel)
    ColorType = 0;        // Bit 1 = Palette used
    // Bit 2 = Color used
    // Bit 3 = Alpha used
    
    EfeWidth = 0;         // Efective Width
    
    lpbi = NULL;
    bgindex = -1;
    m_palette = 0;
    imageOK = FALSE;
}

wxPNGReader::wxPNGReader ( char* ImageFileName )
{
    imageOK = FALSE;
    filetype = 0;
    RawImage = NULL;      //  Image data
    
    Width = 0; Height = 0;       //  Dimensions
    Depth = 0;           // (bits x pixel)
    ColorType = 0;        // Bit 1 = m_palette used
    // Bit 2 = Color used
    // Bit 3 = Alpha used
    
    EfeWidth = 0;         // Efective Width
    
    lpbi = NULL;
    bgindex = -1;
    m_palette = 0;
    
    imageOK = ReadFile (ImageFileName);
}

void
wxPNGReader::Create(int width, int height, int depth, int colortype)
{
    Width = width; Height = height; Depth = depth;
    ColorType = (colortype>=0) ? colortype: ((Depth>8) ? COLORTYPE_COLOR: 0);
    delete m_palette;
    m_palette = NULL;
    delete[] RawImage;
    RawImage = NULL;
    
    if (lpbi) {
        wxMacDestroyGWorld( (GWorldPtr) lpbi ) ;
    }
    lpbi = wxMacCreateGWorld( Width , Height , Depth);
    if (lpbi)
    {
        EfeWidth = (long)(((long)Width*Depth + 31) / 32) * 4;
        int bitwidth = width ;
        if ( EfeWidth > bitwidth )
            bitwidth = EfeWidth ;
        
        RawImage = (byte*) new char[ ( bitwidth * Height * ((Depth+7)>>3) ) ];
        imageOK = TRUE;
    }
}

wxPNGReader::~wxPNGReader ( )
{
    if (RawImage != NULL) {
        delete[] RawImage ;
        RawImage = NULL;
    }
    if (lpbi)  {
        wxMacDestroyGWorld( (GWorldPtr) lpbi ) ;
        lpbi = NULL;
    }
    if (m_palette != NULL) {
        delete m_palette;
        m_palette = NULL;
    }
}


int wxPNGReader::GetIndex(int x, int y)
{
    if (!Inside(x, y) || (Depth>8)) return -1;
    
    ImagePointerType ImagePointer = RawImage + EfeWidth*y + (x*Depth >> 3);
    int index = (int)(*ImagePointer);
    return index;
}

bool wxPNGReader::GetRGB(int x, int y, byte* r, byte* g, byte* b)
{
    if (!Inside(x, y)) return FALSE;
    
    if (m_palette) {
        return m_palette->GetRGB(GetIndex(x, y), r, g, b);
        /*   PALETTEENTRY entry;
        ::GetPaletteEntries((HPALETTE) m_palette->GetHPALETTE(), GetIndex(x, y), 1, &entry);
        *r = entry.peRed;
        *g = entry.peGreen;
        *b = entry.peBlue;  */
    } else {
        ImagePointerType ImagePointer = RawImage + EfeWidth*y + (x*Depth >> 3);
        *b = ImagePointer[0];
        *g = ImagePointer[1];
        *r = ImagePointer[2];
    }
    return TRUE;
}


bool wxPNGReader::SetIndex(int x, int y, int index)
{
    if (!Inside(x, y) || (Depth>8)) return FALSE;
    
    ImagePointerType ImagePointer = RawImage + EfeWidth*y + (x*Depth >> 3);
    *ImagePointer = index;
    
    return TRUE;
}

bool wxPNGReader::SetRGB(int x, int y, byte r, byte g, byte b)
{
    if (!Inside(x, y)) return FALSE;
    
    if (ColorType & COLORTYPE_PALETTE)
    {
        if (!m_palette) return FALSE;
        SetIndex(x, y, m_palette->GetPixel(r, g, b));
        
    } else {
        ImagePointerType ImagePointer = RawImage + EfeWidth*y + (x*Depth >> 3);
        ImagePointer[0] = b;
        ImagePointer[1] = g;
        ImagePointer[2] = r;
    }
    
    return TRUE;
}

bool wxPNGReader::SetPalette(wxPalette* colourmap)
{
    delete m_palette ;
    if (!colourmap)
        return FALSE;
    ColorType |= (COLORTYPE_PALETTE | COLORTYPE_COLOR);
    m_palette = new wxPalette( *colourmap );
    return true ;
    //  return (DibSetUsage(lpbi, (HPALETTE) m_palette->GetHPALETTE(), WXIMA_COLORS ) != 0);
}

bool
wxPNGReader::SetPalette(int n, byte *r, byte *g, byte *b)
{
    delete m_palette ;
    m_palette = new wxPalette();
    if (!m_palette)
        return FALSE;
    
    if (!g) g = r;
    if (!b) b = g;
    m_palette->Create(n, r, g, b);
    ColorType |= (COLORTYPE_PALETTE | COLORTYPE_COLOR);
    return true ;
    //    return (DibSetUsage(lpbi, (HPALETTE) m_palette->GetHPALETTE(), WXIMA_COLORS ) != 0);
}

bool
wxPNGReader::SetPalette(int n, rgb_color_struct *rgb_struct)
{
    delete m_palette ;
    m_palette = new wxPalette();
    if (!m_palette)
        return FALSE;
    
    byte r[256], g[256], b[256];
    
    for(int i=0; i<n; i++)
    {
        r[i] = rgb_struct[i].red;
        g[i] = rgb_struct[i].green;
        b[i] = rgb_struct[i].blue;
    }
    // Added by JACS copying from Andrew Davison's additions
    // to GIF-reading code
    // Make transparency colour black...
    if (bgindex != -1)
        r[bgindex] = g[bgindex] = b[bgindex] = 0;
    
    m_palette->Create(n, r, g, b);
    ColorType |= (COLORTYPE_PALETTE | COLORTYPE_COLOR);
    return true ;
    //    return (DibSetUsage(lpbi, (HPALETTE) m_palette->GetHPALETTE(), WXIMA_COLORS ) != 0);
}

void wxPNGReader::NullData()
{
    if (lpbi)  {
        wxMacDestroyGWorld( (GWorldPtr) lpbi ) ;
        lpbi = NULL;
    }
    if (m_palette != NULL) {
        delete m_palette;
        m_palette = NULL;
    }
}

wxBitmap* wxPNGReader::GetBitmap(void)
{
    wxBitmap *bitmap = new wxBitmap;
    if ( InstantiateBitmap(bitmap) )
        return bitmap;
    else
    {
        delete bitmap;
        return NULL;
    }
}

bool wxPNGReader::InstantiateBitmap(wxBitmap *bitmap)
{
    if ( lpbi )
    {
        bitmap->SetHBITMAP((WXHBITMAP) lpbi);
        bitmap->SetWidth(GetWidth());
        bitmap->SetHeight(GetHeight());
        bitmap->SetDepth(GetDepth());
        if ( GetDepth() > 1 && m_palette )
            bitmap->SetPalette(*m_palette);
        bitmap->SetOk(TRUE);
        
        
        // Make a mask if appropriate
        /*
        if ( bgindex > -1 )
        {
        wxMask *mask = CreateMask();
        bitmap->SetMask(mask);
        }
        */
        lpbi = NULL ; // bitmap has taken over ownership
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    /*
    HDC dc = ::CreateCompatibleDC(NULL);
    
      if (dc)
      {
      // tmpBitmap is a dummy, to satisfy ::CreateCompatibleDC (it
      // is a memory dc that must have a bitmap selected into it)
      HDC dc2 = GetDC(NULL);
      HBITMAP tmpBitmap = ::CreateCompatibleBitmap(dc2, GetWidth(), GetHeight());
      ReleaseDC(NULL, dc2);
      HBITMAP oldBitmap = (HBITMAP) ::SelectObject(dc, tmpBitmap);
      
        if ( m_palette )
        {
        HPALETTE oldPal = ::SelectPalette(dc, (HPALETTE) m_palette->GetHPALETTE(), FALSE);
        ::RealizePalette(dc);
        }
        
          HBITMAP hBitmap = ::CreateDIBitmap(dc, lpbi,
          CBM_INIT, RawImage, (LPBITMAPINFO) lpbi, DIB_PAL_COLORS);
          
            ::SelectPalette(dc, NULL, TRUE);
            ::SelectObject(dc, oldBitmap);
            ::DeleteObject(tmpBitmap);
            ::DeleteDC(dc);
            
              if ( hBitmap )
              {
              bitmap->SetHBITMAP((WXHBITMAP) hBitmap);
              bitmap->SetWidth(GetWidth());
              bitmap->SetHeight(GetHeight());
              bitmap->SetDepth(GetDepth());
              if ( GetDepth() > 1 && m_palette )
              bitmap->SetPalette(*m_palette);
              bitmap->SetOk(TRUE);
              
                
                  // Make a mask if appropriate
                  if ( bgindex > -1 )
                  {
                  wxMask *mask = CreateMask();
                  bitmap->SetMask(mask);
                  }
                  return TRUE;
                  }
                  else
                  {
                  return FALSE;
                  }
                  }
                  else
                  {
                  return FALSE;
                  }
    */
    return false ;
}

wxPalette *wxCopyPalette(const wxPalette *cmap)
{
    wxPalette *newCmap = new wxPalette( *cmap ) ;
    return newCmap;
}

wxMask *wxPNGReader::CreateMask(void)
{
/*
HBITMAP hBitmap = ::CreateBitmap(GetWidth(), GetHeight(), 1, 1, NULL);

  HDC dc = ::CreateCompatibleDC(NULL);
  HBITMAP oldBitmap = (HBITMAP) ::SelectObject(dc, hBitmap);
  
    int bgIndex = GetBGIndex();
    
      int x,y;
      
        for (x=0; x<GetWidth(); x++)
        {
        for (y=0; y<GetHeight(); y++)
        {
        int index = GetIndex(x, y);
        if ( index == bgIndex )
        ::SetPixel(dc, x, GetHeight() - y - 1, RGB(0, 0, 0));
        else
        ::SetPixel(dc, x, GetHeight() - y - 1, RGB(255, 255, 255));
        
          }
          }
          ::SelectObject(dc, oldBitmap);
          wxMask *mask = new wxMask;
          mask->SetMaskBitmap((WXHBITMAP) hBitmap);
          return mask;
    */
    return NULL ;
}

bool wxPNGReader::ReadFile(char * ImageFileName)
{
    int number_passes;
    
    if (ImageFileName)
        strcpy(filename, ImageFileName);
    
    FILE *fp;
    png_struct *png_ptr;
    png_info *info_ptr;
    wxPNGReaderIter iter(this);
    
    /* open the file */
    fp = fopen( ImageFileName , "rb" );
    
    if (!fp)
        return FALSE;
    
    /* allocate the necessary structures */
    png_ptr = new (png_struct);
    if (!png_ptr)
    {
        fclose(fp);
        return FALSE;
    }
    
    info_ptr = new (png_info);
    if (!info_ptr)
    {
        fclose(fp);
        delete png_ptr;
        return FALSE;
    }
    /* set error handling */
    if (setjmp(png_ptr->jmpbuf))
    {
        png_read_destroy(png_ptr, info_ptr, (png_info *)0);
        fclose(fp);
        delete png_ptr;
        delete info_ptr;
        
        /* If we get here, we had a problem reading the file */
        return FALSE;
    }
    //png_set_error(ima_png_error, NULL);
    
    /* initialize the structures, info first for error handling */
    png_info_init(info_ptr);
    png_read_init(png_ptr);
    
    /* set up the input control */
    png_init_io(png_ptr, fp);
    
    /* read the file information */
    png_read_info(png_ptr, info_ptr);
    
    /* allocate the memory to hold the image using the fields
    of png_info. */
    png_color_16 my_background={ 0, 31, 127, 255, 0 };
    
    if (info_ptr->valid & PNG_INFO_bKGD)
    {
        png_set_background(png_ptr, &(info_ptr->background),
            PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
        if ( info_ptr->num_palette > 0 )
            bgindex = info_ptr->background.index;
    }
    else   {
        png_set_background(png_ptr, &my_background,
            PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
        
        // Added by JACS: guesswork!
        if ( info_ptr->num_trans != 0 )
            bgindex = info_ptr->num_trans - 1 ;
    }
    
    /* tell libpng to strip 16 bit depth files down to 8 bits */
    if (info_ptr->bit_depth == 16)
        png_set_strip_16(png_ptr);
    
    int pixel_depth=(info_ptr->pixel_depth<24) ? info_ptr->pixel_depth: 24;
    Create(info_ptr->width, info_ptr->height, pixel_depth,
        info_ptr->color_type);
    
    if (info_ptr->num_palette>0)
    {
        SetPalette((int)info_ptr->num_palette, (rgb_color_struct*)info_ptr->palette);
    }
    
    int row_stride = info_ptr->width * ((pixel_depth+7)>>3);
    //  printf("P = %d D = %d RS= %d ", info_ptr->num_palette, info_ptr->pixel_depth,row_stride);
    //  printf("CT = %d TRS = %d BD= %d ", info_ptr->color_type, info_ptr->valid & PNG_INFO_tRNS,info_ptr->bit_depth);
    
    byte *row_pointers = new byte[row_stride];
    
    /* turn on interlace handling */
    if (info_ptr->interlace_type)
        number_passes = png_set_interlace_handling(png_ptr);
    else
        number_passes = 1;
    //  printf("NP = %d ", number_passes);
    
    for (int pass=0; pass< number_passes; pass++) 
    {
        iter.upset();
        int y=0;
        CGrafPtr     origPort ;
        GDHandle    origDevice ;
        
        GetGWorld( &origPort , &origDevice ) ;
        // ignore shapedc
        SetGWorld( (GWorldPtr) lpbi , NULL ) ;
        do  
        {
            //    (unsigned char *)iter.GetRow();
            if (info_ptr->interlace_type)  
            {
                if (pass>0)
                    iter.GetRow(row_pointers, row_stride);
                png_read_row(png_ptr, row_pointers, NULL);
            }
            else
                png_read_row(png_ptr, row_pointers, NULL);
            
            if ( info_ptr->palette )
            {
                if ( pixel_depth == 8 )
                {
                    for ( size_t i = 0 ; i < info_ptr->width ; ++i )
                    {
                        png_color_struct* color ; 
                        RGBColor col ;
                        
                        int index = row_pointers[i] ;
                        color = &info_ptr->palette[index] ;
                        col.red = (((int)color->red) << 8) | ((int)color->red) ;
                        col.green = (((int)color->green) << 8) | ((int)color->green) ;
                        col.blue = (((int)color->blue) << 8) | ((int)color->blue) ;
                        SetCPixel( i, y, &col);
                    }
                    /*
                    png_color_struct* color ; 
                    RGBColor col ;
                    unsigned char* p = &row_pointers[0] ;
                    PenNormal() ;
                    MoveTo( 0 , y ) ;
                    int index = *p ;
                    color = &info_ptr->palette[index] ;
                    col.red = (color->red << 8) | color->red ;
                    col.green = (color->green << 8) | color->green ;
                    col.blue = (color->blue << 8) | color->blue ;
                    RGBForeColor( &col ) ;
                    col.red = col.green = col.blue = 0xFFFF ;
                    RGBBackColor( &col ) ;
                    for ( int i = 0 ; i < info_ptr->width ; ++i , ++p)
                    {
                    if ( *p != index )
                    {
                    LineTo( i , y ) ;
                    index = *p ;
                    color = &info_ptr->palette[index] ;
                    col.red = (((int)color->red) << 8) | ((int)color->red) ;
                    col.green = (((int)color->green) << 8) | ((int)color->green) ;
                    col.blue = (((int)color->blue) << 8) | ((int)color->blue) ;
                    RGBForeColor( &col ) ;
                    }
                    }
                    LineTo( info_ptr->width  , y ) ;
                    */
                }
                else
                {
                    for ( size_t i = 0 ; i < info_ptr->width ; ++i )
                    {
                        png_color_struct* color ; 
                        RGBColor col ;
                        
                        int byte = ( i * pixel_depth ) / 8 ;
                        int offset = ( 8 - pixel_depth ) - ( i * pixel_depth ) % 8 ;
                        
                        int index = ( row_pointers[byte] >> offset ) & ( 0xFF >> ( 8 - pixel_depth ) );
                        color = &info_ptr->palette[index] ;
                        col.red = (((int)color->red) << 8) | ((int)color->red) ;
                        col.green = (((int)color->green) << 8) | ((int)color->green) ;
                        col.blue = (((int)color->blue) << 8) | ((int)color->blue) ;
                        SetCPixel( i, y, &col);
                    }
                }
            }
            else
            {
                for ( size_t i = 0 ; i < info_ptr->width ; ++i )
                {
                    png_color_struct* color ; 
                    RGBColor col ;
                    color =(png_color_struct*) (&row_pointers[i*3]) ;
                    col.red = (((int)color->red) << 8) | ((int)color->red) ;
                    col.green = (((int)color->green) << 8) | ((int)color->green) ;
                    col.blue = (((int)color->blue) << 8) | ((int)color->blue) ;
                    SetCPixel( i, y, &col);
                }
            }
            if (number_passes)
                iter.SetRow(row_pointers, row_stride);
            y++;
        } 
        while(iter.PrevRow());
        SetGWorld( origPort , origDevice ) ;
        
        //  printf("Y=%d ",y);
  }
  delete[] row_pointers;
  
  /* read the rest of the file, getting any additional chunks
  in info_ptr */
  png_read_end(png_ptr, info_ptr);
  
  /* clean up after the read, and free any memory allocated */
  png_read_destroy(png_ptr, info_ptr, (png_info *)0);
  
  /* free the structures */
  delete png_ptr;
  delete info_ptr;
  
  /* close the file */
  fclose(fp);
  
  /* that's it */
  return TRUE;
}


/* write a png file */

bool wxPNGReader::SaveFile(char * ImageFileName)
{
    if (ImageFileName)
        strcpy(filename, ImageFileName);
    
    wxPNGReaderIter iter(this);
    FILE *fp;
    png_struct *png_ptr;
    png_info *info_ptr;
    
    /* open the file */
    fp = fopen(filename, "wb");
    if (!fp)
        return FALSE;
    
    /* allocate the necessary structures */
    png_ptr = new (png_struct);
    if (!png_ptr)
    {
        fclose(fp);
        return FALSE;
    }
    
    info_ptr = new (png_info);
    if (!info_ptr)
    {
        fclose(fp);
        delete png_ptr;
        return FALSE;
    }
    
    /* set error handling */
    if (setjmp(png_ptr->jmpbuf))
    {
        png_write_destroy(png_ptr);
        fclose(fp);
        delete png_ptr;
        delete info_ptr;
        
        /* If we get here, we had a problem reading the file */
        return FALSE;
    }
    //png_set_error(ima_png_error, NULL);
    
    //  printf("writig pg %s ", filename);
    /* initialize the structures */
    png_info_init(info_ptr);
    png_write_init(png_ptr);
    
    int row_stride = GetWidth() * ((GetDepth()+7)>>3);
    /* set up the output control */
    png_init_io(png_ptr, fp);
    
    /* set the file information here */
    info_ptr->width = GetWidth();
    info_ptr->height = GetHeight();
    info_ptr->pixel_depth = GetDepth();
    info_ptr->channels = (GetDepth()>8) ? 3: 1;
    info_ptr->bit_depth = GetDepth()/info_ptr->channels;
    info_ptr->color_type = GetColorType();
    info_ptr->compression_type = info_ptr->filter_type = info_ptr->interlace_type=0;
    info_ptr->valid = 0;
    info_ptr->rowbytes = row_stride;
    
    
    // printf("P = %d D = %d RS= %d GD= %d CH= %d ", info_ptr->pixel_depth, info_ptr->bit_depth, row_stride, GetDepth(), info_ptr->channels);
    /* set the palette if there is one */
    if ((GetColorType() & COLORTYPE_PALETTE) && GetPalette())
    {
        //    printf("writing paleta[%d %d %x]",GetColorType() ,COLORTYPE_PALETTE, GetPalette());
        info_ptr->valid |= PNG_INFO_PLTE;
        info_ptr->palette = new png_color[256];
        info_ptr->num_palette = 256;
        for (int i=0; i<256; i++)
            GetPalette()->GetRGB(i, &info_ptr->palette[i].red, &info_ptr->palette[i].green, &info_ptr->palette[i].blue);
    }
    //    printf("Paleta [%d %d %x]",GetColorType() ,COLORTYPE_PALETTE, GetPalette());
    
    
    /* optional significant bit chunk */
    //   info_ptr->valid |= PNG_INFO_sBIT;
    //   info_ptr->sig_bit = true_bit_depth;
    
    /* optional gamma chunk */
    //   info_ptr->valid |= PNG_INFO_gAMA;
    //   info_ptr->gamma = gamma;
    
    /* other optional chunks */
    
    /* write the file information */
    png_write_info(png_ptr, info_ptr);
    
    /* set up the transformations you want.  Note that these are
    all optional.  Only call them if you want them */
    
    /* shift the pixels up to a legal bit depth and fill in
    as appropriate to correctly scale the image */
    //   png_set_shift(png_ptr, &(info_ptr->sig_bit));
    
    /* pack pixels into bytes */
    //   png_set_packing(png_ptr);
    
    /* flip bgr pixels to rgb */
    //   png_set_bgr(png_ptr);
    
    /* swap bytes of 16 bit files to most significant bit first */
    //   png_set_swap(png_ptr);
    
    /* get rid of filler bytes, pack rgb into 3 bytes */
    //   png_set_rgbx(png_ptr);
    
    /* If you are only writing one row at a time, this works */
    
    byte *row_pointers = new byte[row_stride];
    iter.upset();
    do  {
        //    (unsigned char *)iter.GetRow();
        iter.GetRow(row_pointers, row_stride);
        png_write_row(png_ptr, row_pointers);
    } while(iter.PrevRow());
    
    delete[] row_pointers;
    
    /* write the rest of the file */
    png_write_end(png_ptr, info_ptr);
    
    /* clean up after the write, and free any memory allocated */
    png_write_destroy(png_ptr);
    
    /* if you malloced the palette, free it here */
    if (info_ptr->palette)
        delete[] (info_ptr->palette);
    
    /* free the structures */
    delete png_ptr;
    delete info_ptr;
    
    /* close the file */
    fclose(fp);
    
    /* that's it */
    return TRUE;
}

static int Power(int x, int y)
{
    int z = 1;
    int i;
    for ( i = 0; i < y; i++)
    {
        z *= x;
    }
    return z;
}

static char hexArray[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B',
'C', 'D', 'E', 'F' };

static void DecToHex(int dec, char *buf)
{
    int firstDigit = (int)(dec/16.0);
    int secondDigit = (int)(dec - (firstDigit*16.0));
    buf[0] = hexArray[firstDigit];
    buf[1] = hexArray[secondDigit];
    buf[2] = 0;
}


bool wxPNGReader::SaveXPM(char *filename, char *name)
{
    char nameStr[256];
    if ( name )
        strcpy(nameStr, name);
    else
    {
    	wxString str = wxString::FromAscii(filename) ;
    	wxStripExtension( str ) ;
        strcpy(nameStr, str.ToAscii() );
    }
    
    if ( GetDepth() > 4 )
    {
        // Only a depth of 4 and below allowed
        return FALSE;
    }
    
    if ( !GetPalette() )
        return FALSE;
    
    wxSTD ofstream str(filename);
    if ( str.bad() )
        return FALSE;
    
    int noColours = Power(2, GetDepth());
    
    // Output header
    str << "/* XPM */\n";
    str << "static char * " << nameStr << "_xpm[] = {\n";
    str << "\"" << GetWidth() << " " << GetHeight() << " " << noColours << " 1\",\n";
    
    // Output colourmap
    int base = 97 ; // start from 'a'
    
    unsigned char red, green, blue;
    char hexBuf[4];
    int i;
    for ( i = 0; i < noColours; i ++)
    {
        str << "\"" << (char)(base + i) << "      c #";
        GetPalette()->GetRGB(i, &red, &green, &blue);
        DecToHex(red, hexBuf);
        str << hexBuf;
        DecToHex(green, hexBuf);
        str << hexBuf;
        DecToHex(blue, hexBuf);
        str << hexBuf;
        str << "\",\n";
    }
    
    // Output the data
    int x, y;
    for ( y = 0; y < GetHeight(); y++)
    {
        str << "\"";
        for ( x = 0; x < GetWidth(); x++)
        {
            int index = GetIndex(x, y);
            str << (char)(base + index) ;
        }
        str << "\",\n";
    }
    
    str << "};\n";
    str.flush();
    
    return TRUE;
}


IMPLEMENT_DYNAMIC_CLASS(wxPNGFileHandler, wxBitmapHandler)

bool wxPNGFileHandler::LoadFile(wxBitmap *bitmap, const wxString& name, long flags,
                                int desiredWidth, int desiredHeight)
{
    wxPNGReader reader;
    if (reader.ReadFile( (char*)(const char*) name.ToAscii() ) )
    {
        return reader.InstantiateBitmap(bitmap);
    }
    else
        return FALSE;
}

bool wxPNGFileHandler::SaveFile(const wxBitmap *bitmap, const wxString& name, int type, const wxPalette *pal)
{
    return FALSE;
}

