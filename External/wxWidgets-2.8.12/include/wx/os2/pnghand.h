/////////////////////////////////////////////////////////////////////////////
// Name:        wx/os2/pnghand.h
// Purpose:     PNG bitmap handler
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: pnghand.h 42077 2006-10-17 14:44:52Z ABX $
// Copyright:   (c) Microsoft, Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PNGHAND_H_
#define _WX_PNGHAND_H_

class WXDLLEXPORT wxPNGFileHandler: public wxBitmapHandler
{
    DECLARE_DYNAMIC_CLASS(wxPNGFileHandler)
public:
    inline wxPNGFileHandler(void)
    {
        m_sName = "PNG bitmap file";
        m_sExtension = "bmp";
        m_lType = wxBITMAP_TYPE_PNG;
    };

    virtual bool LoadFile( wxBitmap*       pBitmap
                          ,const wxString& rName
                          ,HPS             hPs
                          ,long            lFlags
                          ,int             nDesiredWidth
                          ,int             nDesiredHeight
                         );
    virtual bool SaveFile( wxBitmap*        pBitmap
                          ,const wxString&  rName
                          ,int              nType
                          ,const wxPalette* pPalette = NULL
                         );
};

#endif
    // _WX_PNGHAND_H_
