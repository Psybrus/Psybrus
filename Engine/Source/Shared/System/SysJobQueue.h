/**************************************************************************
*
* File:		SysJobQueue.h
* Author:	Neil Richardson 
* Ver/Date:	6/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SysJobQueue_H__
#define __SysJobQueue_H__

#include "Base/BcTypes.h"
#include <mutex>
#include "Base/BcThread.h"
#include "Base/BcMisc.h"

#include "System/SysJob.h"
#include "System/SysJobWorker.h"
#include "System/SysFence.h"

#include <list>
#include <vector>

//////////////////////////////////////////////////////////////////////////
// SysJobQueue
class SysJobQueue:
	public BcThread
{
public:
	SysJobQueue( BcU32 NoofWorkers );
	~SysJobQueue();
	
	/**
	 * Enqueue a job.
	 * @param pJob Job to queue.
	 * @param WorkerMask Used to specify which workers the job is allowed to schedule on.
	 */
	void				enqueueJob( SysJob* pJob, BcU32 WorkerMask = 0xffffffff );
	
	/**
	 * Flush jobs.
	 */
	void				flushJobs();

	/**
	 * Signal a schedule cycle.
	 */
	void				schedule();
	
	/**
	 * Get worker usage mask.
	 */
	BcU32				workerUsageMask() const;

	/**
	 * Get worker count.
	 */
	BcU32				workerCount() const;

	/**
	 * Get and reset time working for worker.
	 */
	BcF32				getAndResetTimeWorkingForWorker( BcU32 Idx );
	
	/**
	 * Get and jobs executed for worker.
	 */
	BcU32				getAndResetJobsExecutedForWorker( BcU32 Idx );

private:
	/**
	 * Move jobs to the back of the queue.
	 * @param WorkerMask Mask to move to the front of the queue.
	 */
	void				moveJobsBack( BcU32 WorkerMask );
	
private:
	virtual void		execute();
	
private:
	typedef std::list< SysJob* > TJobQueue;
	typedef TJobQueue::iterator TJobQueueIterator;
	typedef std::vector< SysJobWorker* > TJobWorkerList;
	
	SysFence			StartedFence_;
	BcBool				Active_;
	BcEvent				ResumeEvent_;
	std::atomic< BcU32 > NoofJobsQueued_;
	BcU32				NoofWorkers_;
	BcU32				AvailibleWorkerMask_;
	
	std::mutex				QueueLock_;
	TJobQueue			JobQueue_;
	
	TJobWorkerList		JobWorkers_;
};

#endif
