/////////////////////////////////////////////////////////////////////////////
// Name:        wx/palmos/mslu.h
// Purpose:     Fixes for bugs in MSLU
// Author:      William Osborne - minimal working wxPalmOS port
// Modified by:
// Created:     10/13/04
// RCS-ID:      $Id: mslu.h 31469 2005-01-18 21:14:27Z ABX $
// Copyright:   (c) William Osborne
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSLU_H_
#define _WX_MSLU_H_

#include "wx/defs.h"
#include "wx/utils.h"
#include "wx/wxchar.h"

#if !wxUSE_UNICODE_MSLU
inline bool wxUsingUnicowsDll() { return false; }
#else

// Returns true if we are running under Unicode emulation in Win9x environment.
// Workaround hacks take effect only if this condition is met
inline bool wxUsingUnicowsDll()
{
    return (wxGetOsVersion() == wxWIN95);
}

//------------------------------------------------------------------------
// Wrongly implemented functions from unicows.dll
//------------------------------------------------------------------------

#if wxUSE_GUI

WXDLLEXPORT int wxMSLU_DrawStateW(WXHDC dc, WXHBRUSH br, WXFARPROC outputFunc,
                                  WXLPARAM lData, WXWPARAM wData,
                                  int x, int y, int cx, int cy,
                                  unsigned int flags);
#define DrawStateW(dc, br, func, ld, wd, x, y, cx, cy, flags) \
    wxMSLU_DrawStateW((WXHDC)dc,(WXHBRUSH)br,(WXFARPROC)func, \
                      ld, wd, x, y, cx, cy, flags)

WXDLLEXPORT int wxMSLU_GetOpenFileNameW(void *ofn);
#define GetOpenFileNameW(ofn) wxMSLU_GetOpenFileNameW((void*)ofn)

WXDLLEXPORT int wxMSLU_GetSaveFileNameW(void *ofn);
#define GetSaveFileNameW(ofn) wxMSLU_GetSaveFileNameW((void*)ofn)

#endif

//------------------------------------------------------------------------
// Missing libc file manipulation functions in Win9x
//------------------------------------------------------------------------

WXDLLEXPORT int wxMSLU__trename(const wxChar *oldname, const wxChar *newname);
WXDLLEXPORT int wxMSLU__tremove(const wxChar *name);

#endif // wxUSE_UNICODE_MSLU

#endif // _WX_MSLU_H_
