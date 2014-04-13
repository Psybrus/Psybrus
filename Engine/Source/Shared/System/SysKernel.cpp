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

#include "System/SysKernel.h"
#include "System/SysProfilerChromeTracing.h"
#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

#if PLATFORM_WINDOWS
#include "Base/BcWindows.h"
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
SysKernel::SysKernel( BcF32 TickRate ):
	JobQueue_( BcMax( BcGetHardwareThreadCount(), BcU32( 1 ) ) ),
	TickRate_( TickRate )
{
	ShuttingDown_ = BcFalse;
	SleepAccumulator_ = 0.0f;
	FrameTime_ = 0.0f;

	// Set user mask to the workers we have.
	SysKernel::USER_WORKER_MASK = ( ( 1 << JobQueue_.workerCount() ) - 1 );
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

			// Set name to what it was instanced from.
			pSystem->setName( Name );
		
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
	BcPrintf( "============================================================================\n" );
	BcPrintf( "SysKernel run\n" );
	BcPrintf( Threaded ? "Threaded.\n" : "Non-threaded.\n" );

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
	PSY_PROFILER_SECTION( TickRoot, "SysKernel::tick" );

	BcAssert( BcIsGameThread() );

#if 0
	// Reset time working in the job queue for metrics.
	BcPrintf( "System Kernel: Game thread: %f ms\n", GameThreadTime_ * 1000.0f );
	for( BcU32 Idx = 0; Idx < JobQueue_.workerCount(); ++Idx )
	{
		BcF32 Time = JobQueue_.getAndResetTimeWorkingForWorker( Idx );
		BcU32 Jobs = JobQueue_.getAndResetJobsExecutedForWorker( Idx );
		BcPrintf( "System Kernel: Worker %u: %f ms (%u jobs)\n", Idx, Time * 1000.0f, Jobs );
	}
#endif

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
			
			//  Process system.
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
BcF32 SysKernel::getFrameTime() const
{
	return FrameTime_;
}

//////////////////////////////////////////////////////////////////////////
// execute
//virtual
void SysKernel::execute()
{
	// Set main thread.
	BcSetGameThread();

#if PSY_USE_PROFILER
	BcU32 FrameCount = 0;
#endif
	
	// Run until there are no more systems to run.
	do
	{
#if PSY_USE_PROFILER
		if( FrameCount == 0 )
		{
			BcProfiler::pImpl()->beginProfiling();
		}
#endif

		// Mark main timer.
		MainTimer_.mark();
		
		// Tick systems.
		tick();

		// Store game thread time.
		GameThreadTime_ = (BcF32)MainTimer_.time();
		
		// Sleep if we have a fixed rate specified, otherwise just yield.
		if( TickRate_ > 0.0f )
		{
			PSY_PROFILER_SECTION( TickSleep, "Sleep" );

			BcF32 TimeSpent = (BcF32)MainTimer_.time();
			SleepAccumulator_ += BcMax( ( TickRate_ ) - TimeSpent, 0.0f );
		
			if( SleepAccumulator_ > 0.0f )
			{
				BcF32 SleepTime = SleepAccumulator_;
				SleepAccumulator_ -= SleepTime;
				BcSleep( BcMin( SleepTime, TickRate_ ) );
			}
		}
		else
		{
			BcYield();
		}

		// Store frame time.
		FrameTime_ = BcMin( (BcF32)MainTimer_.time(), TickRate_ * 4.0f );

		BcAssert( FrameTime_ >= 0.0f );

#if PSY_USE_PROFILER
		++FrameCount;

		if( FrameCount == 9 )
		{
			BcProfiler::pImpl()->endProfiling();
			FrameCount = 0;
		}
#endif
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
