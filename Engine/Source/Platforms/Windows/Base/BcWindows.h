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

#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef NOKANJI
#define NOKANJI
#endif

#ifndef NOHELP
#define NOHELP
#endif

#ifndef NOPROFILER
#define NOPROFILER
#endif

#ifndef NOMCX
#define NOMCX
#endif

#include <winsock2.h>
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

