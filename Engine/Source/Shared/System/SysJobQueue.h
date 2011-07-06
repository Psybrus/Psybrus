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

#include "BcTypes.h"
#include "BcMutex.h"
#include "BcThread.h"
#include "BcScopedLock.h"
#include "SysJob.h"
#include "SysJobWorker.h"

//////////////////////////////////////////////////////////////////////////
// SysJobQueue
class SysJobQueue:
	public BcThread
{
public:
	SysJobQueue( BcU32 NoofWorkers );
	~SysJobQueue();
		
	/**
	 * Queue a job.
	 * @param pJob Job to queue.
	 * @param WorkerMask Used to specify which workers the job is allowed to schedule on.
	 */
	void				queueJob( SysJob* pJob, BcU32 WorkerMask = BcErrorCode );
	
	/**
	 * Flush jobs.
	 */
	void				flushJobs();

	/**
	 * Signal a schedule cycle.
	 */
	void				schedule();
	
private:
	virtual void		execute();
	
private:
	typedef std::deque< SysJob* > TJobQueue;
	typedef std::vector< SysJobWorker* > TJobWorkerList;
	
	BcBool				Active_;
	BcEvent				ResumeEvent_;
	BcAtomicU32			NoofJobsQueued_;
	BcU32				NoofWorkers_;

	BcMutex				QueueLock_;
	TJobQueue			JobQueue_;
	
	TJobWorkerList		JobWorkers_;
};

#endif
