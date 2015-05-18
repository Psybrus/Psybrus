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
#if USE_BOOST_LOCKFREE_QUEUE
	JobQueue_( 32 ),
#endif
	NoofJobs_( 0 )
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
#if USE_BOOST_LOCKFREE_QUEUE
	BcBool RetVal = JobQueue_.push( Job );
#else
	std::lock_guard< std::mutex > Lock( JobQueueMutex_ );

	BcBool RetVal = BcTrue;
	JobQueue_.push_back( Job );
#endif
	if( RetVal )
	{
		++NoofJobs_;
	}
	else
	{
		BcBreakpoint;
	}

	Parent_->notifySchedule();

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// popJob
BcBool SysJobQueue::popJob( SysJob*& Job )
{
#if USE_BOOST_LOCKFREE_QUEUE
	BcBool RetVal = JobQueue_.pop( Job );
#else
	std::lock_guard< std::mutex > Lock( JobQueueMutex_ );

	BcBool RetVal = BcFalse;
	if( JobQueue_.size() > 0 )
	{
		Job = JobQueue_.front();
		BcAssert( Job != nullptr );
		JobQueue_.pop_front();
		RetVal = BcTrue;
	}
#endif

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// completedJob
BcU32 SysJobQueue::completedJob()
{
	return --NoofJobs_;
}

//////////////////////////////////////////////////////////////////////////
// flushJobs
void SysJobQueue::flushJobs( BcBool ForceExecute )
{
	while( anyJobsWaiting() )
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
#if USE_BOOST_LOCKFREE_QUEUE
	BcBool Empty = JobQueue_.empty();
	return !Empty;
#else
	std::lock_guard< std::mutex > Lock( JobQueueMutex_ );

	BcBool Empty = JobQueue_.empty();
	return !Empty;
#endif
}

//////////////////////////////////////////////////////////////////////////
// anyJobsWaiting
BcBool SysJobQueue::anyJobsWaiting()
{
	return NoofJobs_.load() > 0;
}

//////////////////////////////////////////////////////////////////////////
// debugLog
void SysJobQueue::debugLog()
{
	std::lock_guard< std::mutex > Lock( JobQueueMutex_ );

#if !USE_BOOST_LOCKFREE_QUEUE
	PSY_LOG( "JobQueue size: %u", JobQueue_.size() );
#endif
	PSY_LOG( "NoofJobs: %u", NoofJobs_.load() );
}
