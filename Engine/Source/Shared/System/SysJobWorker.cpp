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
SysJobWorker::SysJobWorker( class SysKernel* Parent, SysFence& StartFence ):
	Parent_( Parent ),
	StartFence_( StartFence ),
	Active_( BcTrue ),
	PendingJobQueue_( 0 ),
	JobQueueIndex_( 0 )
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
	if( ExecutionThread_.joinable() )
	{
		ExecutionThread_.join();
	}
}

//////////////////////////////////////////////////////////////////////////
// start
void SysJobWorker::start()
{
	// Mark active.
	Active_ = BcTrue;

#if !PLATFORM_HTML5
	// Just start the thread.
	ExecutionThread_ = std::thread( &SysJobWorker::execute, this );
#else
	BcBreakpoint;
#endif
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
// addJobQueue
void SysJobWorker::addJobQueue( SysJobQueue* JobQueue )
{
	BcAssert( BcIsGameThread() );

	std::lock_guard< std::mutex > Lock( JobQueuesLock_ );
	NextJobQueues_.push_back( JobQueue );
	PendingJobQueue_++;
}

//////////////////////////////////////////////////////////////////////////
// anyJobsWaiting
BcBool SysJobWorker::anyJobsWaiting()
{
	BcBool RetVal = BcFalse;
	for( auto JobQueue : CurrJobQueues_ )
	{
		// Check if job queue has any jobs waiting.
		if( JobQueue->anyJobsWaiting() )
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
	PSY_LOGSCOPEDCATEGORY( "Worker" );

	// Mark as started.
	StartFence_.decrement();

	// Enter loop.
	while( Active_ )
	{
		PSY_PROFILER_SECTION( WaitSchedule_Profiler, "SysJobWorker_WaitSchedule" );

		// Wait to be scheduled.
		Parent_->waitForSchedule( [ this ]()
		{
			const BcBool AnyJobsWaiting = anyJobsWaiting();
			const BcBool PendingJobQueue = PendingJobQueue_.load() > 0;
			return AnyJobsWaiting || PendingJobQueue || !Active_;
		});

		PSY_PROFILER_SECTION( DoneSchedule_Profiler, "SysJobWorker_DoneSchedule" );

		// Check for a job queues update.
		if( PendingJobQueue_.load() > 0 )
		{
			std::lock_guard< std::mutex > Lock( JobQueuesLock_ );
			CurrJobQueues_.insert( CurrJobQueues_.end(), NextJobQueues_.begin(), NextJobQueues_.end() );

			// Wrap job queue index round to fit into new size.
			JobQueueIndex_ = JobQueueIndex_ % CurrJobQueues_.size();

			// Decrement by size of queue.
			PendingJobQueue_.fetch_sub( NextJobQueues_.size() );
			NextJobQueues_.clear();
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
					PSY_LOG( "Unhandled exception in job.\n" );
				}

				// Delete job.
				delete Job;

				// Tell job queue we've completed the job.
				JobQueue->completedJob();

				break;
			}
		}
	}
}
