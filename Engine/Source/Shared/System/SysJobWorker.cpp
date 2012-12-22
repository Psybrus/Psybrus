/**************************************************************************
*
* File:		SysJobWorker.cpp
* Author:	Neil Richardson 
* Ver/Date:	6/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/SysJobWorker.h"
#include "System/SysJobQueue.h"
#include "Base/BcTimer.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
SysJobWorker::SysJobWorker( SysJobQueue* pParent ):
	pParent_( pParent ),
	Active_( BcTrue ),
	HaveJob_( BcFalse ),
	pCurrentJob_( NULL ),
	ResumeEvent_( NULL )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SysJobWorker::~SysJobWorker()
{
	BcAssertMsg( BcThread::isActive() == BcFalse, "SysJobWorker: Not been stopped before destruction." );
}

//////////////////////////////////////////////////////////////////////////
// giveJob
BcBool SysJobWorker::giveJob( SysJob* pJob )
{
	BcBool HaveJob = HaveJob_.compareExchange( BcTrue, BcFalse );
	
	// We don't have a job, we can handle this one.
	if( HaveJob == BcFalse )
	{
		pCurrentJob_ = pJob;
		ResumeEvent_.signal();
	}
	
	return !HaveJob;
}

//////////////////////////////////////////////////////////////////////////
// inUse
BcBool SysJobWorker::inUse() const
{
	return HaveJob_;
}

//////////////////////////////////////////////////////////////////////////
// start
void SysJobWorker::start()
{
	// Just start the thread.
	BcThread::start( "SysJobWorker Main" );
}

//////////////////////////////////////////////////////////////////////////
// stop
void SysJobWorker::stop()
{
	// Set to not be active, trigger resume, and join thread.
	Active_ = BcFalse;
	ResumeEvent_.signal();
	BcThread::join();
}

//////////////////////////////////////////////////////////////////////////
// getAndResetTimeWorking
BcReal SysJobWorker::getAndResetTimeWorking()
{
	BcU32 TimeWorkingUS = TimeWorkingUS_.exchange( 0 );
	return static_cast< BcReal >( TimeWorkingUS ) / 1000000.0f;
}

//////////////////////////////////////////////////////////////////////////
// execute
//virtual
void SysJobWorker::execute()
{
	while( Active_ )
	{
		// Wait till we are told to resume.
		ResumeEvent_.wait();

		if( Active_ == BcTrue )
		{
			BcAssertMsg( pCurrentJob_ != NULL, "No job has been given!" );
		}

		// If we have a job set, we need to execute it.
		if( pCurrentJob_ != NULL )
		{
			BcAssertMsg( HaveJob_ == BcTrue, "SysJobWorker: We have a job pointer set, but haven't got it via giveJob." );

			// Start timing the job.
#if !PSY_PRODUCTION
			BcTimer Timer;
			Timer.mark();
#endif
			// Execute our job.
			pCurrentJob_->internalExecute();

#if !PSY_PRODUCTION
			// Add time spent to our total.
			const BcU32 TimeWorkingUS = static_cast< BcU32 >( Timer.time() * 1000000.0f );;
			TimeWorkingUS_ += TimeWorkingUS;
#endif			
			// No job now, clean up.
			delete pCurrentJob_;
			pCurrentJob_ = NULL;
			HaveJob_ = BcFalse;
			
			// Signal job queue parent to schedule.
			pParent_->schedule();
		}
	}
}
