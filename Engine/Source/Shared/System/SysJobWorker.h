/**************************************************************************
*
* File:		SysJobWorker.h
* Author:	Neil Richardson 
* Ver/Date:	6/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SysJobWorker_H__
#define __SysJobWorker_H__

#include "Base/BcTypes.h"
#include "System/SysJob.h"
#include "System/SysFence.h"
#include "System/SysJobQueue.h"

#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class SysJobQueue;

//////////////////////////////////////////////////////////////////////////
// SysJobWorker
class SysJobWorker
{
public:
	SysJobWorker( class SysKernel* Parent );
	virtual ~SysJobWorker();
	
	/**
	 * Start worker.
	 */
	void					start();
	
	/**
	 * Stop worker.
	 */
	void					stop();

	/**
	 * Update job queues.
	 */
	void					updateJobQueues( SysJobQueueList JobQueues );

	/**
	 * Get current job queue list.
	 */
	SysJobQueueList			getJobQueueList() const;

	/**
	 * Wait for pending job queue list.
	 */
	void					waitForPendingJobQueueList() const;

	/**
	 * Do we have any jobs waiting?
	 */
	BcBool					anyJobsWaiting();

private:
	virtual void			execute();
	
private:
	class SysKernel*		Parent_;
	BcBool					Active_;
	std::atomic< BcU32 >	PendingJobQueue_;
	std::thread				ExecutionThread_;
	std::mutex				JobQueuesLock_;

	SysJobQueueList			NextJobQueues_;
	SysJobQueueList			CurrJobQueues_;
	size_t					JobQueueIndex_;
};

#endif
