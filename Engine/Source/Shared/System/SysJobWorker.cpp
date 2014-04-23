/**************************************************************************
*
* File:		SysJobWorker.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/SysJobWorker.h"
#include "System/SysJobQueue.h"
#include "System/SysKernel.h"
#include "Base/BcTimer.h"
#include "Base/BcProfiler.h"

#include <mutex>

//////////////////////////////////////////////////////////////////////////
// Ctor
SysJobWorker::SysJobWorker( class SysKernel* Parent ):
	Parent_( Parent ),
	Active_( BcTrue ),
	PendingJobQueue_( 0 )
{
	// Start immediately.
	start();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SysJobWorker::~SysJobWorker()
{
	// Stop immediately.
	stop();

	//
	ExecutionThread_.join();
}

//////////////////////////////////////////////////////////////////////////
// start
void SysJobWorker::start()
{
	// Mark active.
	Active_ = BcTrue;

	// Just start the thread.
	ExecutionThread_ = std::thread( &SysJobWorker::execute, this );
}

//////////////////////////////////////////////////////////////////////////
// stop
void SysJobWorker::stop()
{
	// Set to not be active, trigger resume, and join thread.
	Active_ = BcFalse;
	//std::lock_guard< std::mutex > ResumeLock( ResumeMutex_ );
	//ResumeEvent_.notify_all();
	ExecutionThread_.join();
}

//////////////////////////////////////////////////////////////////////////
// updateJobQueues
void SysJobWorker::updateJobQueues( SysJobQueueList JobQueues )
{
	std::lock_guard< std::mutex > Lock( JobQueuesLock_ );
	NextJobQueues_ = std::move( JobQueues );
	PendingJobQueue_++;
}

//////////////////////////////////////////////////////////////////////////
// anyJobsWaiting
BcBool SysJobWorker::anyJobsWaiting()
{
	BcBool RetVal = BcFalse;
	for( auto JobQueue : CurrJobQueues_ )
	{
		// Check if job queue has any jobs pending.
		if( JobQueue->anyJobsPending() )
		{
			RetVal = BcTrue;
			break;
		}
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// execute
//virtual
void SysJobWorker::execute()
{
	// Enter loop.
	while( Active_ )
	{
		// Wait to be scheduled.
		Parent_->waitForSchedule( [ this ]()
			{
				return anyJobsWaiting() || PendingJobQueue_.load() > 0;
			});

		// Check for a job queues update.
		if( PendingJobQueue_.load() > 0 )
		{
			std::lock_guard< std::mutex > Lock( JobQueuesLock_ );
			CurrJobQueues_ = std::move( NextJobQueues_ );

			// Wrap job queue index round to fit into new size.
			JobQueueIndex_ = JobQueueIndex_ % CurrJobQueues_.size();

			// No more pending job queue.
			// NOTE: Safe to reset as we have a lock
			//       on the job queues as it is.
			PendingJobQueue_.store( 0 );
		}

		// Grab job from current job queue.
		SysJob* Job = nullptr;
		for( size_t Idx = 0; Idx < CurrJobQueues_.size(); ++Idx )
		{
			// Grab job queue.
			auto& JobQueue( CurrJobQueues_[ JobQueueIndex_ ] );

			// Advance.
			JobQueueIndex_ = ( JobQueueIndex_ + 1 ) % CurrJobQueues_.size();

			// If we can pop, execute and break out.
			if( JobQueue->popJob( Job ) )
			{
				// Execute.
				Job->internalExecute();
				break;
			}
		}
	}
}
