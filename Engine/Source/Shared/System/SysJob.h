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

#include "BcTypes.h"
#include "BcDebug.h"
#include "BcMutex.h"

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

#endif


