/**************************************************************************
*
* File:		BcLog.cpp
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*		Logging system.
*
*
*
*
**************************************************************************/

#include "Base/BcLog.h"
#include "Base/BcScopedLock.h"
#include "Base/BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// Platform includes.
#if PLATFORM_WINDOWS
#include <windows.h>
#endif
#include <stdio.h>
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////
// Ctor
BcLog::BcLog()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
BcLog::~BcLog()
{
	flush();
}

//////////////////////////////////////////////////////////////////////////
// write
void BcLog::write( const BcChar* pText, ... )
{
	BcScopedLock< BcMutex > Lock( Lock_ );

	va_list ArgList;
	va_start( ArgList, pText );
	privateWrite( pText, ArgList );
	va_end( ArgList );
}

//////////////////////////////////////////////////////////////////////////
// write
void BcLog::write( BcU32 Catagory, const BcChar* pText, ... )
{
	BcScopedLock< BcMutex > Lock( Lock_ );

	if( getCatagorySuppression( Catagory ) == BcFalse )
	{
		va_list ArgList;
		va_start( ArgList, pText );
		privateWrite( pText, ArgList );
		va_end( ArgList );
	}
}

//////////////////////////////////////////////////////////////////////////
// flush
void BcLog::flush()
{
	BcScopedLock< BcMutex > Lock( Lock_ );

	internalFlush();
}

//////////////////////////////////////////////////////////////////////////
// setCatagorySuppression
void BcLog::setCatagorySuppression( BcU32 Catagory, BcBool IsSuppressed )
{
	BcScopedLock< BcMutex > Lock( Lock_ );

	if( IsSuppressed == BcTrue )
	{
		SuppressedMap_[ Catagory ] = IsSuppressed;
	}
	else
	{
		TSuppressionMap::iterator It( SuppressedMap_.find( Catagory ) );
		if( It != SuppressedMap_.end() )
		{
			SuppressedMap_.erase( It );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getCatagorySuppression
BcBool BcLog::getCatagorySuppression( BcU32 Catagory ) const
{
	BcScopedLock< BcMutex > Lock( Lock_ );

	TSuppressionMap::const_iterator It( SuppressedMap_.find( Catagory ) );

	if( It != SuppressedMap_.end() )
	{
		return (*It).second;
	}

	return BcFalse;
}


//////////////////////////////////////////////////////////////////////////
// internalWrite
//virtual
void BcLog::internalWrite( const BcChar* pText )
{
	// Just handle all platforms in here. That way deriving is simpler for everyone.
#if PLATFORM_WINDOWS
	::OutputDebugStringA( pText );
#endif

	// Generic case.
	printf( "%s", pText );

}

//////////////////////////////////////////////////////////////////////////
// internalFlush
//virtual
void BcLog::internalFlush()
{

}

//////////////////////////////////////////////////////////////////////////
// privateWrite
void BcLog::privateWrite( const BcChar* pText, va_list Args )
{
#if COMPILER_MSVC
	vsprintf_s( TextBuffer_, pText, Args );
#else
    vsprintf( TextBuffer_, pText, Args );
#endif
	internalWrite( TextBuffer_ );
}
