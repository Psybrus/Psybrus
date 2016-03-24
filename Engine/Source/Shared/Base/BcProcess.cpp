#include "Base/BcProcess.h"
#include "Base/BcLog.h"

#include <stdio.h>

#include <thread>

std::future< int > BcProcessLaunch(
	const char* Commandline,
	BcProcessLogFunc LogFunc )
{
	std::atomic< int > LaunchedCheck( 0 );
	std::future< int > RetVal = std::async( std::launch::async, 
		[ &LaunchedCheck, Commandline, LogFunc ]()
		{
			FILE* File = popen( Commandline, "r" );
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
			return -1;
		} );
	while( LaunchedCheck == 0 )
	{
		std::this_thread::yield();
	}
	return RetVal;
}
