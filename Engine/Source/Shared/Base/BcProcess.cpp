#include "Base/BcProcess.h"
#include "Base/BcLog.h"

#include <stdio.h>
#include <stdlib.h>

#include <array>
#include <thread>

#if PLATFORM_WINDOWS
#include <Windows.h>
#include <sstream>
#endif

std::future< int > BcProcessLaunch(
	const char* Path,
	const char* Arguments,
	BcProcessLogFunc LogFunc )
{
	std::atomic< int > LaunchedCheck( 0 );
	std::future< int > RetVal = std::async( std::launch::async, 
		[ &LaunchedCheck, Path, Arguments, LogFunc ]()
		{
#if PLATFORM_OSX || PLATFORM_LINUX
			std::string Commandline = Path;
			if( Arguments != nullptr )
			{
				Commandline += " ";
				Commandline += Arguments;
			}
			FILE* File = popen( Commandline.c_str(), "r" );
			LaunchedCheck = 1;
			if( File )
			{
				char* Buffer = nullptr;
				size_t BufferSize = 0;
				ssize_t NumBytes = 0;
				while( ( NumBytes = getline( &Buffer, &BufferSize, File ) )  != -1 )
				{
					LogFunc( Buffer );

					free( Buffer );
					Buffer = nullptr;
				}

				if( Buffer != nullptr )
				{
					free( Buffer );
				}

				return pclose( File );
			}
#elif PLATFORM_WINDOWS
			std::array< char, 32768 > InOutArguments;
			BcStrCopy( InOutArguments.data(), InOutArguments.size(), Arguments );
			std::string ArgumentsStr( Arguments );
			STARTUPINFO StartupInfo;
			memset( &StartupInfo, 0, sizeof( StartupInfo ) );
			SECURITY_ATTRIBUTES SaAttr; 
			SaAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
			SaAttr.bInheritHandle = TRUE; 
			SaAttr.lpSecurityDescriptor = nullptr;
			HANDLE OutputHandle = 0;
			HANDLE ErrorHandle = 0;
			HANDLE InputHandle = 0;
			if( !::CreatePipe( &OutputHandle, &InputHandle, &SaAttr, 0 ) ) 
			{
				return -1;
			}

			if ( !::SetHandleInformation( OutputHandle, HANDLE_FLAG_INHERIT, 0 ) )
			{
				return -1;
			}

			if ( !::DuplicateHandle(
					::GetCurrentProcess(), InputHandle,
					::GetCurrentProcess(), &ErrorHandle, 0,
					TRUE,DUPLICATE_SAME_ACCESS ) )
			{
				return -1;
			}

			StartupInfo.hStdOutput = InputHandle;
			StartupInfo.hStdError = ErrorHandle;
			StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

			PROCESS_INFORMATION ProcessInfo;
			memset( &ProcessInfo, 0, sizeof( ProcessInfo ) );
			BOOL RetVal = ::CreateProcessA(
				Path,
				InOutArguments.data(),
				nullptr, 
				nullptr,
				TRUE,
				CREATE_NO_WINDOW,
				nullptr,
				nullptr,
				&StartupInfo,
				&ProcessInfo );
			::CloseHandle( InputHandle );
			::CloseHandle( ErrorHandle );
			if( RetVal )
			{
				LaunchedCheck = 1;

				std::array< char, 32768 > ReadBuffer;
				DWORD BytesRead = 0;
				std::string OutLogBuffer;
				for(;;)
				{
					ReadBuffer.fill( 0 );
					if( !::ReadFile( OutputHandle, ReadBuffer.data(), ReadBuffer.size() - 1, &BytesRead, NULL ) || BytesRead == 0 )
					{
						auto LastError = ::GetLastError();
						if( LastError ==  ERROR_BROKEN_PIPE )
						{
							break;
						}
					}
					
					if( BytesRead > 0 )
					{
						OutLogBuffer += ReadBuffer.data();
						size_t FoundNewLineIt = std::string::npos;
						do
						{
							// Find first newline.
							FoundNewLineIt = OutLogBuffer.find( '\n' );

							// Send substr prior to callback.
							LogFunc( OutLogBuffer.substr( 0, FoundNewLineIt ).c_str() );

							// Strip last log message.
							OutLogBuffer = OutLogBuffer.substr( FoundNewLineIt + 1 );
						}
						while( FoundNewLineIt != std::string::npos );
					}
				}

				// Spit out last lot of log.
				LogFunc( OutLogBuffer.c_str() );

				::CloseHandle( OutputHandle );
				::WaitForSingleObject( ProcessInfo.hProcess, INFINITE );
				DWORD ExitCode = -1;
				::GetExitCodeProcess( ProcessInfo.hProcess, &ExitCode );
				return static_cast< int >( ExitCode );
			}
#endif
			return -1;
		} );
	while( LaunchedCheck == 0 )
	{
		std::this_thread::yield();
	}
	return RetVal;
}
