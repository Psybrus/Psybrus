/**************************************************************************
*
* File:		SysKernel.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		System kernel
*		
*
*
* 
**************************************************************************/

#include "SysKernel.h"
#include "BcMath.h"

#if PLATFORM_WINDOWS
#include "BcWindows.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Worker masks.
BcU32 SysKernel::SYSTEM_WORKER_MASK = 0x0;
BcU32 SysKernel::USER_WORKER_MASK = 0x0;

//////////////////////////////////////////////////////////////////////////
// Command line
std::string SysArgs_;

//////////////////////////////////////////////////////////////////////////
// Ctor
SysKernel::SysKernel( BcReal TickRate ):
	JobQueue_( BcMax( BcGetHardwareThreadCount(), BcU32( 1 ) ) ),
	TickRate_( TickRate )
{
	ShuttingDown_ = BcFalse;
	SleepAccumulator_ = 0.0f;
	FrameTime_ = 0.0f;

	// Set user mask to the workers we have.
	SysKernel::USER_WORKER_MASK = ( 1 << JobQueue_.workerCount() ) - 1;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
SysKernel::~SysKernel()
{
	// Stop.
	stop();

	// Join.
	BcThread::join();
}

//////////////////////////////////////////////////////////////////////////
// registerSystem
void SysKernel::registerSystem( const BcName& Name, SysSystemCreator creator )
{
	BcScopedLock< BcMutex > Lock( SystemLock_ );

	// Add to creator map.
	SystemCreatorMap_[ Name ] = creator;
}

//////////////////////////////////////////////////////////////////////////
// startSystem
SysSystem* SysKernel::startSystem( const BcName& Name )
{
	BcScopedLock< BcMutex > Lock( SystemLock_ );

	SysSystem* pSystem = NULL;
	
	if( ShuttingDown_ == BcFalse )
	{
		TSystemCreatorMapIterator Iter = SystemCreatorMap_.find( Name );
	
		if( Iter != SystemCreatorMap_.end() )
		{
			// Create system.
			pSystem = (Iter->second)();
		
			// Add to pending list.
			PendingAddSystemList_.push_back( pSystem );
		}
		else
		{
			BcPrintf( "SysKernel: Can't start system \"%s\"\n", (*Name).c_str() );
		}
	}
	return pSystem;
}

//////////////////////////////////////////////////////////////////////////
// stop
void SysKernel::stop()
{
	BcScopedLock< BcMutex > Lock( SystemLock_ );
	
	// Iterate over and process all systems.
	TSystemListReverseIterator Iter = SystemList_.rbegin();
	
	while( Iter != SystemList_.rend() )
	{
		// Cache system.
		SysSystem* pSystem = (*Iter);
		
		// Stop system.
		pSystem->stop();
		
		// Next system.
		++Iter;
	}
	
	ShuttingDown_ = BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// run
void SysKernel::run( BcBool Threaded )
{
	IsThreaded_ = Threaded;
	
	if( Threaded == BcTrue )
	{
		// Add all the systems.
		addSystems();

		// Start up the thread.
		BcThread::start( "SysKernel Main" );
	
		// Wait until all systems have opened.
		BcBool AllOpened = BcFalse;
		while( AllOpened == BcFalse )
		{
			AllOpened = BcTrue;

			TSystemListReverseIterator Iter = SystemList_.rbegin();

			while( Iter != SystemList_.rend() )
			{
				// Cache system.
				SysSystem* pSystem = (*Iter);
			
				// Opened?
				AllOpened &= pSystem->isOpened();
			
				// Next system.
				++Iter;
			}

			BcYield();
		}
	}
	else
	{
		// Or run here.
		execute();		
	}
}

//////////////////////////////////////////////////////////////////////////
// tick
void SysKernel::tick()
{
	BcAssert( BcIsGameThread() );

	if( ShuttingDown_ == BcFalse )
	{
		BcScopedLock< BcMutex > Lock( SystemLock_ );

		// Add systems.
		addSystems();
	
		// Remove systems.	
		removeSystems();
	
		// Iterate over and process all systems.
		TSystemListIterator Iter = SystemList_.begin();
	
		while( Iter != SystemList_.end() && ShuttingDown_ == BcFalse )
		{
			// Cache system.
			SysSystem* pSystem = (*Iter);
		
			// Process system.
			if( pSystem->process() == BcFalse )
			{
				PendingRemoveSystemList_.push_back( pSystem );		
			}
		
			// Next system.
			++Iter;
		}
	}
	else
	{
		BcScopedLock< BcMutex > Lock( SystemLock_ );

		// Iterate over and process all systems.
		TSystemListReverseIterator Iter = SystemList_.rbegin();
		
		if( Iter != SystemList_.rend() )
		{
			// Cache system.
			SysSystem* pSystem = (*Iter);
			
			// Process system.
			if( pSystem->process() == BcFalse )
			{
				PendingRemoveSystemList_.push_back( pSystem );		
			}
		}
		
		// Remove systems.
		removeSystems();
	}
	
	// Dispatch callbacks.
	DelegateDispatcher_.dispatch();
}

//////////////////////////////////////////////////////////////////////////
// workerCount
BcU32 SysKernel::workerCount() const
{
	return JobQueue_.workerCount();
}

//////////////////////////////////////////////////////////////////////////
// addSystems
void SysKernel::enqueueJob( BcU32 WorkerMask, SysJob* pJob )
{
	JobQueue_.enqueueJob( pJob, WorkerMask );
}

//////////////////////////////////////////////////////////////////////////
// getFrameTime
BcReal SysKernel::getFrameTime() const
{
	return FrameTime_;
}

//////////////////////////////////////////////////////////////////////////
// execute
//virtual
void SysKernel::execute()
{
	// Set main thread.
	extern void BcSetGameThread();
	BcSetGameThread();

	// Run until there are no more systems to run.
	do
	{
		// Mark main timer.
		MainTimer_.mark();
		
		// Tick systems.
		tick();
		
		// Sleep if we have a fixed rate specified, otherwise just yield.
		if( TickRate_ > 0.0f )
		{
			BcReal TimeSpent = MainTimer_.time();
			SleepAccumulator_ += BcMax( ( TickRate_ ) - TimeSpent, 0.0f );
		
			if( SleepAccumulator_ > 0.0f )
			{
				BcReal SleepTime = SleepAccumulator_;
				SleepAccumulator_ -= SleepTime;
				BcSleep( BcMin( SleepTime, TickRate_ ) );
			}
		}
		else
		{
			BcYield();
		}

		// Store frame time.
		FrameTime_ = BcMin( MainTimer_.time(), TickRate_ * 4.0f );
	}
	while( SystemList_.size() > 0 );
}

//////////////////////////////////////////////////////////////////////////
// addSystems
void SysKernel::addSystems()
{
	TSystemListIterator Iter = PendingAddSystemList_.begin();
	
	while( Iter != PendingAddSystemList_.end() )
	{
		// Cache system.
		SysSystem* pSystem = (*Iter);

		// Set system's kernel.
		BcAssert( pSystem->pKernel() == NULL );
		pSystem->pKernel( this );

		// Add to executing list.
		SystemList_.push_back( pSystem );
		++Iter;
	}
	
	PendingAddSystemList_.clear();
}

//////////////////////////////////////////////////////////////////////////
// removeSystems
void SysKernel::removeSystems()
{
	//
	TSystemListIterator RemIter = PendingRemoveSystemList_.begin();
	
	while( RemIter != PendingRemoveSystemList_.end() )
	{
		// Cache system to remove.
		SysSystem* pRemSystem = (*RemIter);
		
		// Iterate over system list to find it.
		TSystemListIterator SysIter = SystemList_.begin();
		
		while( SysIter != SystemList_.end() )
		{
			// Cache system.
			SysSystem* pSystem = (*SysIter);
			
			// If we've found it erase it and advance.
			if( pRemSystem == pSystem )
			{
				SystemList_.erase( SysIter );
				break;
			}
			
			++SysIter;
		}
		
		// Flush jobs before deleting a system.
		JobQueue_.flushJobs();
		
		// Delete system.
		delete pRemSystem;
		
		// Advance to next system for removal.
		++RemIter;
	}
	
	PendingRemoveSystemList_.clear();
}
