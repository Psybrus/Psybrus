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

	// Signal parent to notify waiting workers.
	Parent_->notifySchedule();

	// Wait for join.
	ExecutionThread_.join();
}

//////////////////////////////////////////////////////////////////////////
// updateJobQueues
void SysJobWorker::updateJobQueues( SysJobQueueList JobQueues )
{
	BcAssert( BcIsGameThread() );

	std::lock_guard< std::mutex > Lock( JobQueuesLock_ );
	NextJobQueues_ = std::move( JobQueues );
	PendingJobQueue_++;
}

//////////////////////////////////////////////////////////////////////////
// getJobQueueList
SysJobQueueList SysJobWorker::getJobQueueList() const
{
	BcAssert( BcIsGameThread() );

	// Wait until pending job queue is being copied in.
	waitForPendingJobQueueList();

	// Return current.
	return CurrJobQueues_;
}

//////////////////////////////////////////////////////////////////////////
// anyJobsWaiting
void SysJobWorker::waitForPendingJobQueueList() const
{
	while( PendingJobQueue_.load() > 0 )
	{
		std::this_thread::yield();
	}
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
		PSY_PROFILER_SECTION( WaitSchedule_Profiler, "SysJobWorker_WaitSchedule" );

		// Wait to be scheduled.
		Parent_->waitForSchedule( JobQueuesLock_, [ this ]()
			{
				return anyJobsWaiting() || PendingJobQueue_.load() > 0;
			});

		PSY_PROFILER_SECTION( DoneSchedule_Profiler, "SysJobWorker_DoneSchedule" );

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
			auto JobQueue( CurrJobQueues_[ JobQueueIndex_ ] );

			// Advance.
			JobQueueIndex_ = ( JobQueueIndex_ + 1 ) % CurrJobQueues_.size();

			// If we can pop, execute and break out.
			if( JobQueue->popJob( Job ) )
			{
				PSY_PROFILER_SECTION( ExecuteJob_Profiler, "SysJobWorker_ExecuteJob" );

				// Execute.
				try
				{
					Job->internalExecute();
				}
				catch( ... )
				{
					BcPrintf( "Unhandled exception in job.\n" );
				}
				break;
			}
		}
	}
}
