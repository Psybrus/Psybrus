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
SysFence::SysFence( size_t InitialValue ):
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
void SysFence::increment( size_t Value )
{
	Count_.fetch_add( Value );
}

////////////////////////////////////////////////////////////////////////////////
// decrement
void SysFence::decrement( size_t Value )
{
	Count_.fetch_sub( Value );
}

////////////////////////////////////////////////////////////////////////////////
// wait
void SysFence::wait( size_t Value ) const
{
	PSY_PROFILER_SECTION( FenceProfiler, "SysFence:wait" );

	while( Count_ > Value )
	{
		BcYield();
	}
}

////////////////////////////////////////////////////////////////////////////////
// count
size_t SysFence::count() const
{
	return Count_;
}
