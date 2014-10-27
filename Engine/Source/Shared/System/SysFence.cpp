/**************************************************************************
*
* File:		SysFence.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/11	
* Description:
*		Fence for synchronisation of jobs.
*		
*
*
* 
**************************************************************************/

#include "System/SysFence.h"
#include "System/SysKernel.h"

#include "Base/BcProfiler.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
SysFence::SysFence( BcU32 InitialValue ):
	Count_( InitialValue )
{

}

////////////////////////////////////////////////////////////////////////////////
// Dtor
SysFence::~SysFence()
{

}


////////////////////////////////////////////////////////////////////////////////
// increment
void SysFence::increment( BcU32 Value )
{
	Count_.fetch_add( Value );
}

////////////////////////////////////////////////////////////////////////////////
// decrement
void SysFence::decrement( BcU32 Value )
{
	Count_.fetch_sub( Value );
}

////////////////////////////////////////////////////////////////////////////////
// wait
void SysFence::wait( BcU32 Value ) const
{
	PSY_PROFILER_SECTION( FenceProfiler, "SysFence:wait" );

	while( Count_ > Value )
	{
		BcYield();
	}
}

////////////////////////////////////////////////////////////////////////////////
// count
BcU32 SysFence::count() const
{
	return Count_;
}
