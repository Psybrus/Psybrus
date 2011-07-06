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

#include "SysJobQueue.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
SysJobQueue::SysJobQueue( BcU32 NoofWorkers ):
	Active_( BcTrue ),
	ResumeEvent_( "SysJobQueue_ResumeEvent" ),
	NoofJobsQueued_( 0 ),
	NoofWorkers_( NoofWorkers )
{
	// Start our thread.
	BcThread::start();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
SysJobQueue::~SysJobQueue()
{
	// No longer active, shouldn't be receiving any more jobs.
	Active_ = BcFalse;
	
	// Resume thread.
	ResumeEvent_.signal();
	
	// Now join.
	BcThread::join();	
}

//////////////////////////////////////////////////////////////////////////
// queueJob
void SysJobQueue::queueJob( SysJob* pJob, BcU32 WorkerMask )
{
	BcAssertMsg( Active_ == BcTrue, "SysJobQueue: Trying to queue a job when inactive." );
	
	if( WorkerMask != 0 )
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
		// No valid worker mask. Execute it in the same thread.
		pJob->internalExecute();
	}
}

//////////////////////////////////////////////////////////////////////////
// flushJobs
void SysJobQueue::flushJobs()
{
	while( NoofJobsQueued_ != 0 )
	{
		// DIRTY HACK FOR PLATFORM.
		::usleep( 0 );
	}
}

//////////////////////////////////////////////////////////////////////////
// schedule
void SysJobQueue::schedule()
{
	ResumeEvent_.signal();
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
				BcBool BlockedByMask = BcFalse; 
				for( BcU32 Idx = 0; Idx < NoofWorkers_; ++Idx )
				{
					if( pJob->WorkerMask_ & ( 1 << Idx ) )
					{
						SysJobWorker* pWorker = JobWorkers_[ Idx ];
						HasScheduled = pWorker->giveJob( pJob );
						
						if( HasScheduled )
						{
							BcPrintf( "SysJobQueue: Scheduled %p on worker 0x%x\n", pJob, Idx );
							break;
						}
					}
					else
					{
						BlockedByMask = BcTrue;
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
					// If it's blocked by a mask, then put it in the back of the queue.
					// Primitive way to handle it, but means we won't block other jobs
					// unless we have multiple queues.
					// TODO: Shuffle the job back 1 so we can process the next.
					// TODO: This is a tight expensive loop, if we are blocked we should
					//       either sleep, or wait for a schedule signal.
					if( BlockedByMask )
					{
						BcScopedLock< BcMutex > Lock( QueueLock_ );	
						JobQueue_.pop_front();
						JobQueue_.push_back( pJob );
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
