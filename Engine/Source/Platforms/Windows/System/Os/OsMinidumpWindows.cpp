/**************************************************************************
*
* File:		OsMinidumpWin32.cpp
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*		Minidump crash handler.
*		Taken from Game Coding Complete.
*		To work in release, generate a pdb.
*
*
**************************************************************************/

#include "OsMinidumpWindows.h"

#if COMPILER_MSVC

#include "Base/BcDebug.h"
#include "Base/BcLog.h"

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP )(HANDLE hProcess,
										  DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
										  CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
										  CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
										  CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

//////////////////////////////////////////////////////////////////////////
// Ctor
OsMinidumpWindows::OsMinidumpWindows()
{
	// Setup exception handler.
	::SetUnhandledExceptionFilter( handler );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
OsMinidumpWindows::~OsMinidumpWindows()
{

}

////////////////////////////////////////////////////////////////////////
// handler
//static
LONG WINAPI OsMinidumpWindows::handler( struct _EXCEPTION_POINTERS* pExceptionInfo )
{
	return pImpl()->writeMiniDump( pExceptionInfo );
}

//////////////////////////////////////////////////////////////////////////
// writeMiniDump
LONG OsMinidumpWindows::writeMiniDump( _EXCEPTION_POINTERS* pExceptionInfo )
{
	PSY_LOG( "============================================================================\n" );
	PSY_LOG( "OsMinidumpWindows:\n" );
	PSY_LOG( " - Psybrus has hit an unhandled exception. Attempting to write mini dump.\n" );

	LONG RetVal = EXCEPTION_CONTINUE_SEARCH;

	pExceptionInfo_ = pExceptionInfo;

	// Find dbghelp.dll.
	HMODULE hDll = NULL;
	CHAR DbgHelpPath[ _MAX_PATH ];

	if( GetModuleFileName( NULL, AppPath_, _MAX_PATH ) )
	{
		CHAR* pSlash = strrchr( AppPath_, '\\' );

		if( pSlash )
		{
			strcpy( AppBaseName_, pSlash + 1 );
			*(pSlash + 1 ) = 0;
		}

		strcpy( DbgHelpPath, AppPath_ );
		strcat( DbgHelpPath, "DBGHELP.DLL" );
		hDll = ::LoadLibrary( DbgHelpPath );

		strcpy( DumpPath_, AppPath_ );
		strcat( DumpPath_, AppBaseName_ );
		strcat( DumpPath_, ".dmp" );
	}

	if( hDll == NULL )
	{
		hDll = ::LoadLibrary( "DBGHELP.DLL" );
	}

	LPCTSTR Result = NULL;

	if( hDll )
	{
		MINIDUMPWRITEDUMP pMiniDumpWriteDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );

		if( pMiniDumpWriteDump )
		{
			CHAR Scratch[ _MAX_PATH ];

			//if( ::MessageBox( NULL, _T( "There was an unexpected error, would you like to save a minidump?" ), NULL, MB_YESNO | MB_ICONWARNING ) == IDYES )
			{
				HANDLE hFile = ::CreateFile( DumpPath_, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

				if( hFile != INVALID_HANDLE_VALUE )
				{
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

					ExInfo.ThreadId = ::GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;

					// Write dump.
					BOOL bOk = pMiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );

					if( bOk )
					{
						Result = NULL;
						RetVal = EXCEPTION_EXECUTE_HANDLER;
					}
					else
					{
						sprintf_s( Scratch, "Failed to save dump file to '%s' (error %d)", DumpPath_, GetLastError() );
						Result = Scratch;
					}

					::CloseHandle( hFile );
				}
				else
				{
					sprintf_s( Scratch, "Failed to create dump file '%s' (error %d)", DumpPath_, GetLastError() );
					Result = Scratch;
				}
			}
		}
		else
		{
			Result = "DBGHELP.DLL too old.";
		}
	}
	else
	{
		Result = "DBGHELP.DLL not found";
	}

	if( Result == NULL )
	{
		::MessageBox( NULL, "Psybrus Crashed. Please send the saved \".dmp\" to one of the developers.", "ERROR", MB_OK | MB_ICONSTOP );
	}
	else
	{
		::MessageBox( NULL, Result, "ERROR", MB_OK | MB_ICONSTOP );
	}

	// Flush log and close down.
	BcLog::pImpl()->flush();
	delete BcLog::pImpl();

	//
	::TerminateProcess( GetCurrentProcess(), 1 );

	return RetVal;
}

#endif
