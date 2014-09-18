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

#include <functional>

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
	friend class SysKernel;
	
	void				internalExecute();
	
	BcU32				WorkerMask_;
	
};

//////////////////////////////////////////////////////////////////////////
// SysDelegateJob
class SysDelegateJob:
	public SysJob
{
public:
	SysDelegateJob( class BcDelegateCallBase* pDelegateCall );
	virtual ~SysDelegateJob();
	
	virtual void execute();
	
private:
	class BcDelegateCallBase* pDelegateCall_;
	
};

//////////////////////////////////////////////////////////////////////////
// SysFunctionJob
class SysFunctionJob :
	public SysJob
{
public:
	SysFunctionJob( std::function< void() > Function );
	virtual ~SysFunctionJob();

	virtual void execute();

private:
	std::function< void() > Function_;

};


#endif


