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
	BcAssert( Count_.load() == 0 );
}

////////////////////////////////////////////////////////////////////////////////
// increment
size_t SysFence::increment( size_t Value )
{
	return Count_.fetch_add( Value );
}

////////////////////////////////////////////////////////////////////////////////
// decrement
size_t SysFence::decrement( size_t Value )
{
	return Count_.fetch_sub( Value );
}

////////////////////////////////////////////////////////////////////////////////
// wait
void SysFence::wait( size_t Value, const char* DebugName ) const
{
	PSY_PROFILER_SECTION( DebugName );

	while( Count_ > Value )
	{
		SysKernel::pImpl()->notifySchedule();
		BcYield();
	}
}

////////////////////////////////////////////////////////////////////////////////
// count
size_t SysFence::count() const
{
	return Count_;
}
