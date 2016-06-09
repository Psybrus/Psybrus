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

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

#if PLATFORM_ANDROID
#include <android_native_app_glue.h>
#include <android/log.h>

#define  LOG_TAG    "Psybrus"
#endif

#define EXTENDED_LOG_OUTPUT ( 0 )

//////////////////////////////////////////////////////////////////////////
// Colours.
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#if PLATFORM_WINDOWS
#define WIN_COLOR_RED     FOREGROUND_RED 
#define WIN_COLOR_GREEN   FOREGROUND_GREEN
#define WIN_COLOR_YELLOW  ( FOREGROUND_RED | FOREGROUND_GREEN )
#define WIN_COLOR_BLUE    FOREGROUND_BLUE
#define WIN_COLOR_MAGENTA ( FOREGROUND_RED | FOREGROUND_BLUE )
#define WIN_COLOR_CYAN    ( FOREGROUND_GREEN | FOREGROUND_BLUE )
#define WIN_COLOR_RESET   FOREGROUND_INTENSITY
#endif

//////////////////////////////////////////////////////////////////////////
// BcLogListenerDefault
class BcLogListenerDefault:
	public BcLogListener
{
public:
	BcLogListenerDefault()
	{
	}

private:
	void onLog( const BcLogEntry& Entry )
	{
		std::lock_guard< std::recursive_mutex > Lock( Lock_ );

		// Grab indent.
		std::string Indent( Entry.Indent_, '\t' );

		auto NewText = Entry.Text_;

		// Replace newlines with spaces.
		std::replace( NewText.begin(), NewText.end(), '\n', ' ' );

		static BcChar OutputBuffer[ 1024 * 64 ] = { 0 };
		// Format for output.
#if EXTENDED_LOG_OUTPUT
		BcSPrintf( OutputBuffer, sizeof( OutputBuffer ) - 1, "[%6.3f][%x][%s] %s %s\n", 
			Entry.Time_,
			Entry.ThreadId_,
			(*Entry.Category_).c_str(),
			Indent.c_str(),
			NewText.c_str() );
#else
		BcSPrintf( OutputBuffer, sizeof( OutputBuffer ) - 1, "[%6.3f][%s] %s %s\n", 
			Entry.Time_,
			(*Entry.Category_).c_str(),
			Indent.c_str(),
			NewText.c_str() );
#endif
		internalWrite( OutputBuffer );
	}

	void internalWrite( const BcChar* pText )
	{
		// Just handle all platforms in here. That way deriving is simpler for everyone.
#if PLATFORM_WINDOWS
		HANDLE HConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		::OutputDebugStringA( pText );
#endif

		// Do some colour markup for important information.
		if( BcStrStr( pText, "ERROR:" ) || BcStrStr( pText, "error:" ) || BcStrStr( pText, "Error," ) ||
			BcStrStr( pText, "FAILED:" ) || BcStrStr( pText, "FAILURE:" ) || BcStrStr( pText, "error C" ) )
		{
#if PLATFORM_WINDOWS
			SetConsoleTextAttribute( HConsole, WIN_COLOR_RED );
			printf( "%s", pText );
#elif PLATFORM_ANDROID
			__android_log_print( ANDROID_LOG_ERROR, LOG_TAG, pText );
#else
			printf( ANSI_COLOR_RED "%s", pText );
#endif
		}
		else if( BcStrStr( pText, "SUCCESS:" ) || BcStrStr( pText, "SUCCEEDED:" ) )
		{
#if PLATFORM_WINDOWS
			SetConsoleTextAttribute( HConsole, WIN_COLOR_GREEN );
			printf( "%s", pText );
#elif PLATFORM_ANDROID
			__android_log_print( ANDROID_LOG_INFO, LOG_TAG, pText );
#else
			printf( ANSI_COLOR_GREEN "%s", pText );
#endif
		}
		else if( BcStrStr( pText, "WARNING:" ) || BcStrStr( pText, "warning:" ) || BcStrStr( pText, "Warning," ) )
		{
#if PLATFORM_WINDOWS
			SetConsoleTextAttribute( HConsole, WIN_COLOR_YELLOW );
			printf( "%s", pText );
#elif PLATFORM_ANDROID
			__android_log_print( ANDROID_LOG_WARN, LOG_TAG, pText );
#else
			printf( ANSI_COLOR_YELLOW "%s", pText );
#endif
		}
		else if( BcStrStr( pText, "INFO:" ) || BcStrStr( pText, "info:" ) ||
			BcStrStr( pText, "NOTE:" ) || BcStrStr( pText, "note:" ) )
		{
#if PLATFORM_WINDOWS
			SetConsoleTextAttribute( HConsole, WIN_COLOR_CYAN );
			printf( "%s", pText );
#elif PLATFORM_ANDROID
			__android_log_print( ANDROID_LOG_INFO, LOG_TAG, pText );
#else
			printf( ANSI_COLOR_CYAN "%s", pText );
#endif
		}
		else
		{
#if PLATFORM_WINDOWS
			SetConsoleTextAttribute( HConsole, WIN_COLOR_RESET );
			printf( "%s", pText );
#elif PLATFORM_ANDROID
			__android_log_print( ANDROID_LOG_INFO, LOG_TAG, pText );
#else
			printf( ANSI_COLOR_RESET "%s", pText );
#endif
		}

		// Generic case.
	}

private:
	mutable std::recursive_mutex Lock_;
};

//////////////////////////////////////////////////////////////////////////
// Defines
#define DEFAULT_Category "LOG"

//////////////////////////////////////////////////////////////////////////
// Ctor
BcLogImpl::BcLogImpl()
{
	Timer_.mark();

	DefaultListener_.reset( new BcLogListenerDefault() );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
BcLogImpl::~BcLogImpl()
{
	flush();
	DefaultListener_.reset();
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
}

//////////////////////////////////////////////////////////////////////////
// setCategorySuppression
void BcLogImpl::setCategorySuppression( const BcName Category, BcBool IsSuppressed )
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
BcBool BcLogImpl::getCategorySuppression( const BcName Category ) const
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
// registerListener
void BcLogImpl::registerListener( class BcLogListener* Listener )
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );
	BcAssert( std::find( Listeners_.begin(), Listeners_.end(), Listener ) == Listeners_.end() );
	Listeners_.push_back( Listener );
}

//////////////////////////////////////////////////////////////////////////
// deregisterListener
void BcLogImpl::deregisterListener( class BcLogListener* Listener )
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );
	Listeners_.erase( std::find( Listeners_.begin(), Listeners_.end(), Listener ) );
	BcAssert( std::find( Listeners_.begin(), Listeners_.end(), Listener ) == Listeners_.end() );
}

//////////////////////////////////////////////////////////////////////////
// setCategory
void BcLogImpl::setCategory( const BcName Category )
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );
	auto ThreadId = BcCurrentThreadId();
	Categories_[ ThreadId ] = Category;
}

//////////////////////////////////////////////////////////////////////////
// getCategory
BcName BcLogImpl::getCategory()
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );
	auto ThreadId = BcCurrentThreadId();
	if( Categories_.find( ThreadId ) == Categories_.end() )
	{
		Categories_[ ThreadId ] = DEFAULT_Category;
	}
	return Categories_[ ThreadId ];
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
// privateWrite
void BcLogImpl::privateWrite( const BcChar* pText, va_list Args )
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );
	static BcChar TextBuffer[ 1024 * 64 ];
	auto ThreadId = BcCurrentThreadId();

	BcName Category = Categories_[ ThreadId ];

	if( getCategorySuppression( Category ) == BcFalse )
	{
	#if COMPILER_MSVC
		vsprintf_s( TextBuffer, pText, Args );
	#else
		vsprintf( TextBuffer, pText, Args );
	#endif

		// Construct log entry.
		BcLogEntry Entry = 
		{
			Timer_.time(),
			BcCurrentThreadId(),
			Category,
			IndentLevel_[ ThreadId ],
			TextBuffer
		};

		// Send to all listeners.
		for( auto Listener : Listeners_ )
		{
			Listener->onLog( Entry );
		}
	}
}
