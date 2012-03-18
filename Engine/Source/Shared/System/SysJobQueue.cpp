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

//////////////////////////////////////////////////////////////////////////
// Ctor
SysJobQueue::SysJobQueue( BcU32 NoofWorkers ):
	Active_( BcTrue ),
	ResumeEvent_( NULL ),
	NoofJobsQueued_( 0 ),
	NoofWorkers_( NoofWorkers ),
	AvailibleWorkerMask_( ( 1 << NoofWorkers ) - 1 )
{
	if( NoofWorkers_ > 0 )
	{
		// Start our thread.
		BcThread::start( "SysJobQueue Main" );
	}
	else
	{
		Active_ = BcFalse;
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
SysJobQueue::~SysJobQueue()
{
	if( NoofWorkers_ > 0 )
	{
		// No longer active, shouldn't be receiving any more jobs.
		Active_ = BcFalse;
		
		// Resume thread.
		ResumeEvent_.signal();
		
		// Now join.
		BcThread::join();	
	}
}

//////////////////////////////////////////////////////////////////////////
// enqueueJob
void SysJobQueue::enqueueJob( SysJob* pJob, BcU32 WorkerMask )
{
	BcAssertMsg( Active_ == BcTrue, "SysJobQueue: Trying to queue a job when inactive." );
	
	// Check mask validity and queue if we can.
	if( ( WorkerMask & AvailibleWorkerMask_ ) != 0 )
	{
		BcScopedLock< BcMutex > Lock( QueueLock_ );
	
		// Setup worker mask.
		pJob->WorkerMask_ = WorkerMask;
	
		// Put in queue.
		JobQueue_.push_back( pJob );
	
		// Increment number of jobs queued.
		++NoofJobsQueued_;
	
		// Signal a schedule.
		schedule();
	}
	else
	{
		// No valid worker mask or no workers. Execute it in the same thread as calling.
		pJob->internalExecute();
	}
}

//////////////////////////////////////////////////////////////////////////
// flushJobs
void SysJobQueue::flushJobs()
{
	while( NoofJobsQueued_ != 0 )
	{
		BcYield();
	}
}

//////////////////////////////////////////////////////////////////////////
// schedule
void SysJobQueue::schedule()
{
	ResumeEvent_.signal();
}

//////////////////////////////////////////////////////////////////////////
// workerUsageMask
BcU32 SysJobQueue::workerUsageMask() const
{
	BcU32 UsageMask = 0x0;
	for( BcU32 Idx = 0; Idx < NoofWorkers_; ++Idx )
	{
		SysJobWorker* pWorker = JobWorkers_[ Idx ];
		UsageMask |= pWorker->inUse() ? ( 1 << Idx ) : 0;
	}
	
	return UsageMask;
}

//////////////////////////////////////////////////////////////////////////
// workerCount
BcU32 SysJobQueue::workerCount() const
{
	return NoofWorkers_;
}

//////////////////////////////////////////////////////////////////////////
// moveJobsBack
void SysJobQueue::moveJobsBack( BcU32 WorkerMask )
{
	BcScopedLock< BcMutex > Lock( QueueLock_ ); // NOTE: Have a pending add queue instead of shared global one to prevent this.

	// Remove all jobs which don't fit specified worker mask, and put aside.
	// NOTE: Doing it backwards means we only need one splice, and the fact is
	//       most rendering, sound, and file IO will be locked to single workers
	//       to keep execution in order. This means we copy less around.
	TJobQueue NewJobQueue;
	for( TJobQueueIterator It( JobQueue_.begin() ); It != JobQueue_.end(); )
	{
		SysJob* pJob = (*It);
		
		// The mask matches perfectly, put it into another list and remove it.
		if( pJob->WorkerMask_ == WorkerMask )
		{
			NewJobQueue.push_back( pJob );
			It = JobQueue_.erase( It );
		}
		else
		{
			++It;
		}
	}
	
	// Splice new list into the job queue.
	JobQueue_.splice( JobQueue_.end(), NewJobQueue );
}

//////////////////////////////////////////////////////////////////////////
// execute
//virtual
void SysJobQueue::execute()
{
	// Create & start worker threads.
	for( BcU32 Idx = 0; Idx < NoofWorkers_; ++Idx )
	{
		SysJobWorker* pWorker = new SysJobWorker( this );
		JobWorkers_.push_back( pWorker );
		pWorker->start();
	}

	//
	while( Active_ )
	{
		// Wait for resume event.
		ResumeEvent_.wait();

		// If we've got some jobs queued, enter the scheduling loop.
		if( NoofJobsQueued_ != 0 )
		{
			do
			{
				SysJob* pJob = NULL;

				// Grab job at front of queue.
				{
					BcScopedLock< BcMutex > Lock( QueueLock_ );	
					pJob = JobQueue_.front();
				}

				// Attempt to schedule.
				BcBool HasScheduled = BcFalse;
				BcU32 BlockedMask = 0x0; 
				for( BcU32 Idx = 0; Idx < NoofWorkers_; ++Idx )
				{
					BcU32 CurrMask = ( 1 << Idx );
					if( pJob->WorkerMask_ & CurrMask )
					{
						SysJobWorker* pWorker = JobWorkers_[ Idx ];
						HasScheduled = pWorker->giveJob( pJob );
						
						if( HasScheduled )
						{
							//BcPrintf( "SysJobQueue: Scheduled %p on worker 0x%x\n", pJob, Idx );
							break;
						}
						else
						{
							BlockedMask |= CurrMask;
						}
					}
				}
				
				// If we've scheduled, pop it off.
				if( HasScheduled )
				{
					BcScopedLock< BcMutex > Lock( QueueLock_ );	
					JobQueue_.pop_front();
					
					// Count down number of jobs queued.
					--NoofJobsQueued_;
				}
				else
				{
					// If a mask is blocked, move all jobs which have an exact match to the
					// blocked mask to the back of the queue to prevent contention.
					// This means jobs queued by specific systems with particular worker masks
					// will also keep their order. Differing masks can't possibly keep the same
					// order.
					if( BlockedMask != 0x0 )
					{
						moveJobsBack( BlockedMask );
						BcYield();
					}
				}
			}
			while( NoofJobsQueued_ != 0 );
		}
	}
	
	// Stop and destroy worker threads.
	for( BcU32 Idx = 0; Idx < NoofWorkers_; ++Idx )
	{
		SysJobWorker* pWorker = JobWorkers_[ Idx ];
		
		pWorker->stop();
		delete pWorker;
	}

}
