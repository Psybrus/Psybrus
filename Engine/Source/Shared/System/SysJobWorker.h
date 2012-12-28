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
#include "Base/BcAtomic.h"
#include "Base/BcEvent.h"
#include "Base/BcMutex.h"
#include "Base/BcThread.h"
#include "System/SysJob.h"

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
	 * Reset time working.
	 */
	BcF32				getAndResetTimeWorking();

private:
	virtual void		execute();
	
private:
	SysJobQueue*		pParent_;
	BcBool				Active_;
	BcAtomicBool		HaveJob_;
	SysJob*				pCurrentJob_;
	BcEvent				ResumeEvent_;

	BcAtomicU32			TimeWorkingUS_;	// Microseconds.
};

#endif
