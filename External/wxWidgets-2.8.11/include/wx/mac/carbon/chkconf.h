/*
 * Name:        wx/mac/chkconf.h
 * Purpose:     Compiler-specific configuration checking
 * Author:      Julian Smart
 * Modified by:
 * Created:     01/02/97
 * RCS-ID:      $Id: chkconf.h 48804 2007-09-19 14:50:20Z SC $
 * Copyright:   (c) Julian Smart
 * Licence:     wxWindows licence
 */

#ifndef _WX_MAC_CHKCONF_H_
#define _WX_MAC_CHKCONF_H_

/*
 * disable the settings which don't work for some compilers
 */

#if defined(__MWERKS__)
    #undef wxUSE_DEBUG_NEW_ALWAYS
    #define wxUSE_DEBUG_NEW_ALWAYS      0

    /* DS: Fixes compilation when wxUSE_FATAL_EXCEPTION is 1 */
    #ifndef wxTYPE_SA_HANDLER
        #define wxTYPE_SA_HANDLER int
    #endif
#endif

/*
 * use OS X CoreGraphics (1) or QuickDraw (0) for rendering
 */

#ifndef wxMAC_USE_CORE_GRAPHICS
    #define wxMAC_USE_CORE_GRAPHICS 1
#endif

/*
 * wxMAC_USE_CORE_GRAPHICS is now implemented in terms of wxUSE_GRAPHICS_CONTEXT
 */
 
#if wxMAC_USE_CORE_GRAPHICS 
	#undef wxUSE_GRAPHICS_CONTEXT
	#define wxUSE_GRAPHICS_CONTEXT 1
#endif

/*
 * check graphics context option
 */
#if wxUSE_GRAPHICS_CONTEXT && !wxMAC_USE_CORE_GRAPHICS
#   error "wxUSE_GRAPHICS_CONTEXT on wxMac requires wxMAC_USE_CORE_GRAPHICS"
#endif

/*
 * native (1) or emulated (0) toolbar
 */

#ifndef wxMAC_USE_NATIVE_TOOLBAR
    #define wxMAC_USE_NATIVE_TOOLBAR 1
#endif

/*
 * using mixins of cocoa functionality
 */
 
#ifndef wxMAC_USE_COCOA
    #define wxMAC_USE_COCOA 0
#endif

#endif
    /* _WX_MAC_CHKCONF_H_ */

