/**************************************************************************
*
* File:		SysJob.cpp
* Author:	Neil Richardson 
* Ver/Date:	6/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/SysJob.h"

#include "Base/BcDelegate.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
SysJob::SysJob():
	WorkerMask_( 0 )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
// virtual
SysJob::~SysJob()
{
	
}

//////////////////////////////////////////////////////////////////////////
// internalExecute
// virtual
void SysJob::internalExecute()
{
	execute();
	delete this;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
SysDelegateJob::SysDelegateJob( BcDelegateCallBase* pDelegateCall ):
	pDelegateCall_( pDelegateCall )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SysDelegateJob::~SysDelegateJob()
{
	delete pDelegateCall_;
}
	
//////////////////////////////////////////////////////////////////////////
// execute
//virtual
void SysDelegateJob::execute()
{
	(*pDelegateCall_)();
}
