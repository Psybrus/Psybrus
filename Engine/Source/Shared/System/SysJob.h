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
#include <atomic>

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
	static std::atomic< size_t > FunctionJobID_;
	size_t JobID_;
};


#endif


