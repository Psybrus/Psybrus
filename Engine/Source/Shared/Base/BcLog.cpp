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

//////////////////////////////////////////////////////////////////////////
// Ctor
BcLogListener::BcLogListener()
{
	if( BcLog::pImpl() )
	{
		BcLog::pImpl()->registerListener( this );
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcLogListener::BcLogListener( const BcLogListener& Other )
{
	if( BcLog::pImpl() )
	{
		BcLog::pImpl()->registerListener( this );
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
BcLogListener::~BcLogListener()
{
	if( BcLog::pImpl() )
	{
		BcLog::pImpl()->deregisterListener( this );
	}
}


//////////////////////////////////////////////////////////////////////////
// Ctor
BcLogScopedCategory::BcLogScopedCategory( const BcName Category )
{
	if( BcLog::pImpl() )
	{
		OldCategory_ = BcLog::pImpl()->getCategory();
		BcLog::pImpl()->setCategory( Category );
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcLogScopedCategory::~BcLogScopedCategory()
{
	if( BcLog::pImpl() )
	{
		BcLog::pImpl()->setCategory( OldCategory_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcLogScopedIndent::BcLogScopedIndent()
{
	if( BcLog::pImpl() )
	{
		BcLog::pImpl()->increaseIndent();
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcLogScopedIndent::~BcLogScopedIndent()
{
	if( BcLog::pImpl() )
	{
		BcLog::pImpl()->decreaseIndent();
	}
}
