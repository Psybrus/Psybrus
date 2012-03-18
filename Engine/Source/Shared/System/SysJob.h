/**************************************************************************
*
* File:		SysJob.h
* Author:	Neil Richardson 
* Ver/Date:	6/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SysJob_H__
#define __SysJob_H__

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"
#include "Base/BcMutex.h"
#include "Base/BcDelegate.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations.
class SysJobQueue;
class SysJobWorker;

//////////////////////////////////////////////////////////////////////////
// SysJob
class SysJob
{
public:
	SysJob();
	virtual ~SysJob();
	
	virtual void execute() = 0;
	
private:
	friend class SysJobQueue;
	friend class SysJobWorker;
	
	void				internalExecute();
	
	BcU32				WorkerMask_;
	
};

//////////////////////////////////////////////////////////////////////////
// SysDelegateJob
class SysDelegateJob:
	public SysJob
{
public:
	SysDelegateJob( BcDelegateCallBase* pDelegateCall );
	virtual ~SysDelegateJob();
	
	virtual void execute();
	
private:
	BcDelegateCallBase* pDelegateCall_;
	
};


#endif


