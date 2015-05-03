/**************************************************************************
*
* File:		OsWindows.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OSWINDOWS_H__
#define __OSWINDOWS_H__

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NOMCX

#include <winsock2.h>
#include <windows.h>

// Ditch shitty windows defines.
#undef GetObject
#define GetObject GetObject

#undef CreateDialog
#define CreateDialog CreateDialog

#undef SendMessage
#define SendMessage SendMessage

#undef min
#define min min

#undef max
#define max max

#endif

