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

//////////////////////////////////////////////////////////////////////////
// Platform includes.
#if PLATFORM_WINDOWS
#include <windows.h>
#endif
#include <stdio.h>
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////
// Ctor
BcLogImpl::BcLogImpl()
{

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
	std::lock_guard< std::mutex > Lock( Lock_ );

	va_list ArgList;
	va_start( ArgList, pText );
	privateWrite( pText, ArgList );
	va_end( ArgList );
}

//////////////////////////////////////////////////////////////////////////
// write
void BcLogImpl::write( BcU32 Catagory, const BcChar* pText, ... )
{
	std::lock_guard< std::mutex > Lock( Lock_ );

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
void BcLogImpl::flush()
{
	std::lock_guard< std::mutex > Lock( Lock_ );

	internalFlush();
}

//////////////////////////////////////////////////////////////////////////
// setCatagorySuppression
void BcLogImpl::setCatagorySuppression( BcU32 Catagory, BcBool IsSuppressed )
{
	std::lock_guard< std::mutex > Lock( Lock_ );

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
BcBool BcLogImpl::getCatagorySuppression( BcU32 Catagory ) const
{
	std::lock_guard< std::mutex > Lock( Lock_ );

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
void BcLogImpl::internalWrite( const BcChar* pText )
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
void BcLogImpl::internalFlush()
{

}

//////////////////////////////////////////////////////////////////////////
// privateWrite
void BcLogImpl::privateWrite( const BcChar* pText, va_list Args )
{
#if COMPILER_MSVC
	vsprintf_s( TextBuffer_, pText, Args );
#else
    vsprintf( TextBuffer_, pText, Args );
#endif
	internalWrite( TextBuffer_ );
}
