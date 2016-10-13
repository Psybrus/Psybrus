/**************************************************************************
*
* File:		OsMinidumpWindows.h
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*		Minidump crash handler.
*
*
*
*
**************************************************************************/

#ifndef __OsMinidumpWindows__
#define __OsMinidumpWindows__

#include "Base/BcPortability.h"

#if COMPILER_MSVC && PLATFORM_WINDOWS

#include "OsWindows.h"
#include "Base/BcGlobal.h"

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#pragma warning (push)
#pragma warning (disable : 4091) // 'typedef ': ignored on left of '' when no variable is declared
#include <DbgHelp.h>
#pragma warning (pop)

//////////////////////////////////////////////////////////////////////////
// OsMinidumpWin32
class OsMinidumpWindows:
	public BcGlobal< OsMinidumpWindows >
{
public:
	OsMinidumpWindows();
	~OsMinidumpWindows();

	static LONG WINAPI			handler( struct _EXCEPTION_POINTERS* pExceptionInfo );
	LONG						writeMiniDump( _EXCEPTION_POINTERS* pExceptionInfo );

private:
	_EXCEPTION_POINTERS*		pExceptionInfo_;

	CHAR						DumpPath_[ _MAX_PATH ];
	CHAR						AppPath_[ _MAX_PATH ];
	CHAR						AppBaseName_[ _MAX_PATH ];
};


#endif
#endif
