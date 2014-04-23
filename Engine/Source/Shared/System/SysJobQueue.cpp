/**************************************************************************
*
* File:		SysJobQueue.cpp
* Author:	Neil Richardson 
* Ver/Date:	6/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/SysJobQueue.h"

#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
SysJobQueue::SysJobQueue():
	JobQueue_( 32 )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
SysJobQueue::~SysJobQueue()
{
	
}

//////////////////////////////////////////////////////////////////////////
// pushJob
BcBool SysJobQueue::pushJob( SysJob* Job )
{
	return JobQueue_.push( Job );
}

//////////////////////////////////////////////////////////////////////////
// popJob
BcBool SysJobQueue::popJob( SysJob*& Job )
{
	return JobQueue_.pop( Job );
}

//////////////////////////////////////////////////////////////////////////
// flushJobs
void SysJobQueue::flushJobs( BcBool ForceExecute )
{
	while( anyJobsPending() )
	{
		if( ForceExecute )
		{
			SysJob* Job = nullptr;
			if( popJob( Job ) )
			{
				Job->internalExecute();
			}
		}

		std::this_thread::yield();
	}
}

//////////////////////////////////////////////////////////////////////////
// anyJobsPending
BcBool SysJobQueue::anyJobsPending()
{
	return !JobQueue_.empty();
}