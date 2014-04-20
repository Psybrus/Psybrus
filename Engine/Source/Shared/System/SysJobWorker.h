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
#include "Base/BcThread.h"
#include "System/SysJob.h"
#include "System/SysFence.h"

#include <atomic>
#include <condition_variable>
#include <mutex>

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class SysJobQueue;

//////////////////////////////////////////////////////////////////////////
// SysJobWorker
class SysJobWorker:
	public BcThread
{
public:
	SysJobWorker( SysJobQueue* pParent );
	virtual ~SysJobWorker();
	
	/**
	 * Give a job. Should only be called from one specific thread.
	 * @param pJob Job to execute.
	 * @param Returns BcTrue if succeeded.
	 */
	BcBool				giveJob( SysJob* pJob );
	
	/**
	 * Is worker in use?
	 * Only used for performance and utilisation checking.
	 */
	BcBool				inUse() const;
	
	/**
	 * Start worker.
	 */
	void				start();
	
	/**
	 * Stop worker.
	 */
	void				stop();

	/**
	 * Get and reset time working.
	 */
	BcF32				getAndResetTimeWorking();

	/**
	 * Get and reset jobs executed.
	 */
	BcU32				getAndResetJobsExecuted();

private:
	virtual void		execute();
	
private:
	SysJobQueue*			pParent_;
	BcBool					Active_;
	std::atomic< BcU32 >	HaveJob_;
	SysJob*					pCurrentJob_;
	std::condition_variable	ResumeEvent_;
	std::mutex				ResumeMutex_;
	SysFence				StartFence_;

	std::atomic< BcU32 >	TimeWorkingUS_;	// Microseconds.
	std::atomic< BcU32 >	JobsExecuted_;
};

#endif
