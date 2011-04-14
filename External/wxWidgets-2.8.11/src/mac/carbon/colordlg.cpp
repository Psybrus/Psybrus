/////////////////////////////////////////////////////////////////////////////
// Name:        colordlg.cpp
// Purpose:     wxColourDialog class. NOTE: you can use the generic class
//              if you wish, instead of implementing this.
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: colordlg.cpp 38952 2006-04-29 19:19:58Z DS $
// Copyright:   (c) Stefan Csomor
// Licence:       wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "wx/mac/colordlg.h"
#include "wx/fontdlg.h"


#if !USE_NATIVE_FONT_DIALOG_FOR_MACOSX

IMPLEMENT_DYNAMIC_CLASS(wxColourDialog, wxDialog)

#include "wx/mac/private.h"
#ifndef __DARWIN__
#include <ColorPicker.h>
#endif

/*
 * wxColourDialog
 */

wxColourDialog::wxColourDialog()
{
    m_dialogParent = NULL;
}

wxColourDialog::wxColourDialog(wxWindow *parent, wxColourData *data)
{
    Create(parent, data);
}

bool wxColourDialog::Create(wxWindow *parent, wxColourData *data)
{
    m_dialogParent = parent;

    if (data)
        m_colourData = *data;
    return true;
}

int wxColourDialog::ShowModal()
{
    RGBColor currentColor = *((RGBColor*)m_colourData.m_dataColour.GetPixel()) ;

#if TARGET_API_MAC_OSX
    NColorPickerInfo info;
    OSStatus err ;
    memset(&info, 0, sizeof(info)) ;
    // TODO : use parent to determine better position and then kAtSpecifiedOrigin
    info.placeWhere = kCenterOnMainScreen ; 
    info.flags = kColorPickerDialogIsMoveable | kColorPickerDialogIsModal ;
    info.theColor.color.rgb.red =  currentColor.red ;
    info.theColor.color.rgb.green =  currentColor.green ;
    info.theColor.color.rgb.blue =  currentColor.blue ;
    err = NPickColor(&info);
    if ((err == noErr) && info.newColorChosen)
    {
        currentColor.red = info.theColor.color.rgb.red ;
        currentColor.green = info.theColor.color.rgb.green ;
        currentColor.blue = info.theColor.color.rgb.blue ;
        m_colourData.m_dataColour.FromRGBColor((WXCOLORREF*) &currentColor);

        return wxID_OK;
    }
#else
    RGBColor newColor ;
    Point where ;

    where.h = where.v = -1;

    if (GetColor( where, "\pSelect a new palette color.", &currentColor, &newColor ))
    {
        m_colourData.m_dataColour.Set( (WXCOLORREF*) &newColor ) ;
        return wxID_OK;
    }
#endif
    return wxID_CANCEL;
}

#endif
