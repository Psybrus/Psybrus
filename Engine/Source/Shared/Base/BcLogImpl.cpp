/**************************************************************************
*
* File:		BcLogImpl.cpp
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*		Logging system.
*
*
*
*
**************************************************************************/

#include "Base/BcLogImpl.h"

#include "Base/BcDebug.h"

#include <chrono>

//////////////////////////////////////////////////////////////////////////
// Platform includes.
#if PLATFORM_WINDOWS
#include <windows.h>
#endif
#include <stdio.h>
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////
// Colours.
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//////////////////////////////////////////////////////////////////////////
// Defines
#define DEFAULT_Category "LOG"

//////////////////////////////////////////////////////////////////////////
// Ctor
BcLogImpl::BcLogImpl()
{
	Timer_.mark();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
BcLogImpl::~BcLogImpl()
{
	flush();
}

//////////////////////////////////////////////////////////////////////////
// write
void BcLogImpl::write( const BcChar* pText, ... )
{
	va_list ArgList;
	va_start( ArgList, pText );
	privateWrite( pText, ArgList );
	va_end( ArgList );
}

//////////////////////////////////////////////////////////////////////////
// flush
void BcLogImpl::flush()
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );

	internalFlush();
}

//////////////////////////////////////////////////////////////////////////
// setCategorySuppression
void BcLogImpl::setCategorySuppression( BcName Category, BcBool IsSuppressed )
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );

	if( IsSuppressed == BcTrue )
	{
		SuppressedMap_[ Category ] = IsSuppressed;
	}
	else
	{
		TSuppressionMap::iterator It( SuppressedMap_.find( Category ) );
		if( It != SuppressedMap_.end() )
		{
			SuppressedMap_.erase( It );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getCategorySuppression
BcBool BcLogImpl::getCategorySuppression( BcName Category ) const
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );

	TSuppressionMap::const_iterator It( SuppressedMap_.find( Category ) );

	if( It != SuppressedMap_.end() )
	{
		return (*It).second;
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// setCategory
void BcLogImpl::setCategory( BcName Category )
{

	std::lock_guard< std::recursive_mutex > Lock( Lock_ );
	auto ThreadId = BcCurrentThreadId();
	Catagories_[ ThreadId ] = Category;
}

//////////////////////////////////////////////////////////////////////////
// getCategory
BcName BcLogImpl::getCategory()
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );
	auto ThreadId = BcCurrentThreadId();
	if( Catagories_.find( ThreadId ) == Catagories_.end() )
	{
		Catagories_[ ThreadId ] = DEFAULT_Category;
	}
	return Catagories_[ ThreadId ];
}

//////////////////////////////////////////////////////////////////////////
// getCategorySuppression
void BcLogImpl::increaseIndent()
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );
	IndentLevel_[ BcCurrentThreadId() ]++;
}

//////////////////////////////////////////////////////////////////////////
// getCategorySuppression
void BcLogImpl::decreaseIndent()
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );
	IndentLevel_[ BcCurrentThreadId() ]--;
}

//////////////////////////////////////////////////////////////////////////
// internalWrite
//virtual
void BcLogImpl::internalWrite( const BcChar* pText )
{
	// Just handle all platforms in here. That way deriving is simpler for everyone.
#if PLATFORM_WINDOWS
	::OutputDebugStringA( pText );
#endif

	// Do some colour markup for important information.
	if( BcStrStr( pText, "ERROR:" ) || BcStrStr( pText, "error:" ) || BcStrStr( pText, "Error," ) ||
		BcStrStr( pText, "FAILED:" ) || BcStrStr( pText, "FAILURE:" ) )
	{
		printf( ANSI_COLOR_RED "%s", pText );
	}
	else if( BcStrStr( pText, "SUCCESS:" ) || BcStrStr( pText, "SUCCEEDED:" ) )
	{
		printf( ANSI_COLOR_GREEN "%s", pText );
	}
	else if( BcStrStr( pText, "WARNING:" ) || BcStrStr( pText, "warning:" ) || BcStrStr( pText, "Warning," ) )
	{
		printf( ANSI_COLOR_YELLOW "%s", pText );
	}
	else if( BcStrStr( pText, "INFO:" ) || BcStrStr( pText, "info:" ) ||
		BcStrStr( pText, "NOTE:" ) || BcStrStr( pText, "note:" ) )
	{
		printf( ANSI_COLOR_CYAN "%s", pText );
	}
	else
	{
		printf( ANSI_COLOR_RESET "%s", pText );
	}

	// Generic case.
}

//////////////////////////////////////////////////////////////////////////
// internalFlush
//virtual
void BcLogImpl::internalFlush()
{

}

//////////////////////////////////////////////////////////////////////////
// privateWrite
void BcLogImpl::privateWrite( const BcChar* pText, va_list Args )
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );
	static BcChar TextBuffer[ 1024 * 64 ];
	static BcChar OutputBuffer[ 1024 * 64 ];
	auto ThreadId = BcCurrentThreadId();

	BcName Category = Catagories_[ ThreadId ];

	if( getCategorySuppression( Category ) == BcFalse )
	{
	#if COMPILER_MSVC
		vsprintf_s( TextBuffer, pText, Args );
	#else
		vsprintf( TextBuffer, pText, Args );
	#endif

		// Grab indent.
		std::string Indent( IndentLevel_[ ThreadId ], '\t' );

		// Replace newlines with spaces.
		std::replace( TextBuffer, TextBuffer + BcStrLength( TextBuffer ), '\n', ' ' );


		// Format for output.
		BcSPrintf( OutputBuffer, "[%5.5f][%s] %s %s\n", 
			Timer_.time(),
			(*Category).c_str(),
			Indent.c_str(),
			TextBuffer );

		// Store in internal buffer.
		if ( LogBuffer.size() == 30 )
		{
			LogBuffer.pop_front();
		}
		LogBuffer.push_back( OutputBuffer );

		// Write, platform/target specific impl goes here.
		internalWrite( OutputBuffer );
	}
}

//////////////////////////////////////////////////////////////////////////
// getLogData
std::vector< std::string > BcLogImpl::getLogData()
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );
	
	std::vector< std::string > Data;
	for(const auto& U : LogBuffer)
	{
		Data.push_back( U );
	}
	return Data;
}
