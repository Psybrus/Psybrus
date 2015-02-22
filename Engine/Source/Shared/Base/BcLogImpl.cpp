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

		static BcChar OutputBuffer[ 1024 * 64 ];
		// Format for output.
		BcSPrintf( OutputBuffer, "[%5.5f][%x][%s] %s %s\n", 
			Entry.Time_,
			Entry.ThreadId_,
			(*Entry.Category_).c_str(),
			Indent.c_str(),
			NewText.c_str() );

		internalWrite( OutputBuffer );
	}

	void internalWrite( const BcChar* pText )
	{
		// Just handle all platforms in here. That way deriving is simpler for everyone.
#if PLATFORM_WINDOWS
		::OutputDebugStringA( pText );
#endif

		// TODO: Regex capture of errors.

		// Do some colour markup for important information.
		if( BcStrStr( pText, "ERROR:" ) || BcStrStr( pText, "error:" ) || BcStrStr( pText, "Error," ) ||
			BcStrStr( pText, "FAILED:" ) || BcStrStr( pText, "FAILURE:" ) || BcStrStr( pText, "error C" ) )
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
void BcLogImpl::setCategory( BcName Category )
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

//////////////////////////////////////////////////////////////////////////
// getLogData
std::vector< std::string > BcLogImpl::getLogData()
{
	std::lock_guard< std::recursive_mutex > Lock( Lock_ );
	return std::vector< std::string >();
}
