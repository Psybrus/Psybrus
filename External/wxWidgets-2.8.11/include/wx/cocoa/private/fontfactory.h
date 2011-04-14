/////////////////////////////////////////////////////////////////////////////
// Name:        wx/cocoa/fontfactory.h
// Purpose:     wxFontFactory class
// Author:      David Elliott
// Modified by:
// Created:     2007-10-13
// RCS-ID:      $Id: fontfactory.h 60554 2009-05-08 23:10:00Z VZ $
// Copyright:   2007 David Elliott
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COCOA_FONTFACTORY_H_
#define _WX_COCOA_FONTFACTORY_H_

/*! @discussion
    This class exists so that wxFont need not know about wxSettings
    The implementation, however, is in the font.mm file because the
    implementation needs to have knowledge of wxFontRefData.
 */
class wxCocoaFontFactory
{
public:
    /*! @abstract   Create an instance with the specified NSFont and underline flag.
     */
    static wxFont InstanceForNSFont(WX_NSFont cocoaFont, bool underlined = false);
};

#endif  //_WX_COCOA_FONTFACTORY_H_
