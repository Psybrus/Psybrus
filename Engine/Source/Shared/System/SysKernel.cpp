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
#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

#include "System/SysProfilerChromeTracing.h"
#include "System/SysJobQueue.h"
#include "System/SysJobWorker.h"

#if PLATFORM_WINDOWS
#include "Base/BcWindows.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_BASE( SysKernel );

void SysKernel::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "SystemList_",			&SysKernel::SystemList_ ),
		ReField( "ShuttingDown_",		&SysKernel::ShuttingDown_ ),
		ReField( "IsThreaded_",			&SysKernel::IsThreaded_ ),
		ReField( "MainTimer_",			&SysKernel::MainTimer_ ),
		ReField( "SleepAccumulator_",	&SysKernel::SleepAccumulator_ ),
		ReField( "TickRate_",			&SysKernel::TickRate_ ),
		ReField( "FrameTime_",			&SysKernel::FrameTime_ ),
		ReField( "GameThreadTime_",		&SysKernel::GameThreadTime_ ),
	};
		
	ReRegisterClass< SysKernel >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Worker masks.
BcU32 SysKernel::DEFAULT_JOB_QUEUE_ID = BcErrorCode;

//////////////////////////////////////////////////////////////////////////
// Command line
std::string SysArgs_;

//////////////////////////////////////////////////////////////////////////
// Ctor
SysKernel::SysKernel( ReNoInit )
{
	BcBreakpoint; // Shouldn't hit here ever.
}

//////////////////////////////////////////////////////////////////////////
// Ctor
SysKernel::SysKernel( BcF32 TickRate ):
	TickRate_( TickRate )
{
	ShuttingDown_ = BcFalse;
	SleepAccumulator_ = 0.0f;
	FrameTime_ = 0.0f;
	CurrWorkerAllocIdx_ = 0;

	// Create job workers for the number of threads we have.
	BcU32 NoofThreads = BcMax( std::thread::hardware_concurrency(), 1 );
	JobWorkers_.reserve( NoofThreads );
	for( BcU32 Idx = 0; Idx < NoofThreads; ++Idx )
	{
		JobWorkers_.push_back( new SysJobWorker( this ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
SysKernel::~SysKernel()
{
	// Stop.
	stop();

	// Join.
	ExecutionThread_.join();
	
	// Free job workers.
	for( auto JobWorker : JobWorkers_ )
	{
		delete JobWorker;
	}
	JobWorkers_.clear();

	// Free job queues.
	for( auto JobQueue : JobQueues_ )
	{
		delete JobQueue;
	}
	JobQueues_.clear();
}

//////////////////////////////////////////////////////////////////////////
// createJobQueue
BcU32 SysKernel::createJobQueue( BcU32 NoofWorkers, BcU32 MinimumHardwareThreads )
{
	BcAssertMsg( BcIsGameThread(), "Should only create job queues on the game thread." );

	// Number of workers 0? Then create as many as there are workers.
	if( NoofWorkers == 0 )
	{
		NoofWorkers = JobWorkers_.size();
	}

	// If we have less than the minimum required number of hardware threads,
	// then we just want to return an error code.
	if( NoofWorkers == 0 || MinimumHardwareThreads > std::thread::hardware_concurrency() )
	{
		return BcErrorCode;
	}
	else
	{
		MinimumHardwareThreads = JobWorkers_.size();
	}

	// Clamp number of workers.
	NoofWorkers = BcMin( NoofWorkers, MinimumHardwareThreads );

	auto JobQueue = new SysJobQueue( this );
	JobQueues_.push_back( JobQueue );

	// Add new job queue to the workers.
	for( BcU32 Idx = 0; Idx < NoofWorkers; ++Idx )
	{
		BcU32 RealIdx = ( CurrWorkerAllocIdx_ + Idx ) % JobWorkers_.size();
		auto JobWorker( JobWorkers_[ RealIdx ] );
		auto JobQueueList = JobWorker->getJobQueueList();
		JobQueueList.push_back( JobQueue );
		JobWorker->updateJobQueues( JobQueueList );
	}

	CurrWorkerAllocIdx_ = ( CurrWorkerAllocIdx_ + NoofWorkers ) % JobWorkers_.size(); 

	// Kick off workers so they pick up new job queue.
	notifySchedule();

	return (BcU32)JobQueues_.size() - 1;
}

//////////////////////////////////////////////////////////////////////////
// registerSystem
void SysKernel::registerSystem( const BcName& Name, SysSystemCreator creator )
{
	std::lock_guard< std::recursive_mutex > Lock( SystemLock_ );

	// Add to creator map.
	SystemCreatorMap_[ Name ] = creator;
}

//////////////////////////////////////////////////////////////////////////
// startSystem
SysSystem* SysKernel::startSystem( const BcName& Name )
{
	std::lock_guard< std::recursive_mutex > Lock( SystemLock_ );

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
	std::lock_guard< std::recursive_mutex > Lock( SystemLock_ );
	
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
		ExecutionThread_ = std::thread( &SysKernel::execute, this );
	
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

	if( ShuttingDown_ == BcFalse )
	{
		std::lock_guard< std::recursive_mutex > Lock( SystemLock_ );

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
		std::lock_guard< std::recursive_mutex > Lock( SystemLock_ );

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
	return (BcU32)JobWorkers_.size();
}

//////////////////////////////////////////////////////////////////////////
// addSystems
BcBool SysKernel::pushJob( BcU32 JobQueueId, SysJob* pJob )
{
	// Check if we're out of range.
	if( JobQueueId < JobQueues_.size() )
	{
		return JobQueues_[ JobQueueId ]->pushJob( pJob );
	}

	// No queue we can use, execute on this thread.
	pJob->internalExecute();
	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// flushJobQueue
void SysKernel::flushJobQueue( BcU32 JobQueueId )
{
	// Check if we're out of range.
	if( JobQueueId < JobQueues_.size() )
	{
		return JobQueues_[ JobQueueId ]->flushJobs( BcFalse );
	}
}

//////////////////////////////////////////////////////////////////////////
// flushAllJobQueues
void SysKernel::flushAllJobQueues()
{
	for( auto JobQueue : JobQueues_ )
	{
		JobQueue->flushJobs( BcFalse );
	}
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

		if( FrameCount == 60 )
		{
			BcProfiler::pImpl()->endProfiling();
			FrameCount = 0;
		}
#endif
	}
	while( SystemList_.size() > 0 );
}

//////////////////////////////////////////////////////////////////////////
// notifySchedule
void SysKernel::notifySchedule()
{
	std::lock_guard< std::mutex > Lock( JobQueuedMutex_ );
	JobQueued_.notify_all();
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
		JobQueues_[ 0 ]->flushJobs( BcFalse );
		
		// Delete system.
		delete pRemSystem;
		
		// Advance to next system for removal.
		++RemIter;
	}
	
	PendingRemoveSystemList_.clear();
}
