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
SysFence::SysFence():
	Count_( 0 )
{

}

////////////////////////////////////////////////////////////////////////////////
// Dtor
SysFence::~SysFence()
{

}


////////////////////////////////////////////////////////////////////////////////
// increment
void SysFence::increment()
{
	++Count_;
}

////////////////////////////////////////////////////////////////////////////////
// decrement
void SysFence::decrement()
{
	--Count_;
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
