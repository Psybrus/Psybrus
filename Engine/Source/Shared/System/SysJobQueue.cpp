/**************************************************************************
*
* File:		SysJobQueue.cpp
* Author:	Neil Richardson 
* Ver/Date:	6/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/SysJobQueue.h"
#include "System/SysKernel.h"

#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
SysJobQueue::SysJobQueue( class SysKernel* Parent ):
	Parent_( Parent ),
	JobQueue_( 32 )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
SysJobQueue::~SysJobQueue()
{
	
}

//////////////////////////////////////////////////////////////////////////
// pushJob
BcBool SysJobQueue::pushJob( SysJob* Job )
{
	BcBool RetVal = JobQueue_.push( Job );
	if( RetVal )
	{
		++NoofJobs_;
	}
	Parent_->notifySchedule();
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// popJob
BcBool SysJobQueue::popJob( SysJob*& Job )
{
	BcBool RetVal = JobQueue_.pop( Job );
	if( RetVal )
	{
		--NoofJobs_;
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// flushJobs
void SysJobQueue::flushJobs( BcBool ForceExecute )
{
	while( anyJobsPending() )
	{
		if( ForceExecute )
		{
			SysJob* Job = nullptr;
			if( popJob( Job ) )
			{
				Job->internalExecute();
			}
		}

		Parent_->notifySchedule();
		std::this_thread::yield();
	}
}

//////////////////////////////////////////////////////////////////////////
// anyJobsPending
BcBool SysJobQueue::anyJobsPending()
{
	BcBool Empty = JobQueue_.empty();
	return !Empty;
}
