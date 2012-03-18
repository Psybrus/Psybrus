/**************************************************************************
*
* File:		BcWindows.h
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __BCWINDOWS_H__
#define __BCWINDOWS_H__

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NOMCX

#include <windows.h>

// Undefine windows defines which may conflict within code base.
#undef GetObject
#define GetObject GetObject

#undef CreateDialog
#define CreateDialog CreateDialog

#undef min
#define min min

#undef max
#define max max

#endif

