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
#include "Base/BcRandom.h"

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
	ReField* Fields[] = 
	{
		new ReField( "SystemList_",			&SysKernel::SystemList_ ),
		new ReField( "ShuttingDown_",		&SysKernel::ShuttingDown_ ),
		new ReField( "IsThreaded_",			&SysKernel::IsThreaded_ ),
		new ReField( "MainTimer_",			&SysKernel::MainTimer_ ),
		new ReField( "SleepAccumulator_",	&SysKernel::SleepAccumulator_ ),
		new ReField( "TickRate_",			&SysKernel::TickRate_ ),
		new ReField( "FrameTime_",			&SysKernel::FrameTime_ ),
		new ReField( "GameThreadTime_",		&SysKernel::GameThreadTime_ ),
	};
		
	ReRegisterClass< SysKernel >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// SysKernel_UnitTest
void SysKernel_UnitTest()
{
	SysKernel Kernel;

	// Setup job queues.
	size_t JobQueues[] = 
	{
		Kernel.createJobQueue( 1, 1 ),
		Kernel.createJobQueue( 2, 1 ),
		Kernel.createJobQueue( 3, 1 ),
		Kernel.createJobQueue( 4, 1 )
	};

	std::atomic< BcU32 > IncDecAtomic( 0 );

	auto TestIncJob = [ &IncDecAtomic ]()
	{
		BcSleep( 0.005f );
		++IncDecAtomic;
	};

	auto TestDecJob = [ &IncDecAtomic ]()
	{
		BcSleep( 0.006f );
		--IncDecAtomic;
	};

	BcU32 NoofJobs = 100;
	BcF64 TotalTime = 0.0f;
	BcF64 ThisTime = 0.0f;
	BcTimer Timer;

	// Test no queues.
	BcPrintf( "Begin: Main thread\n" );
	Timer.mark();
	for( BcU32 JobIdx = 0; JobIdx < NoofJobs; ++JobIdx )
	{
		Kernel.pushFunctionJob( (size_t)-1, TestIncJob );
		Kernel.pushFunctionJob( (size_t)-1, TestDecJob );
	}
	Kernel.flushAllJobQueues();
	ThisTime = Timer.time() * 1000.0f;
	TotalTime += ThisTime;
	BcPrintf( "Time: %fms, (%fms total)\n", ThisTime, TotalTime );
	BcUnitTestMsg( IncDecAtomic.load( std::memory_order_seq_cst ) == 0, "Main thread test." );


	// Test each queue individually.
	for( BcU32 QueueIdx = 0; QueueIdx < 4; ++QueueIdx )
	{
		BcPrintf( "Begin: Workers %u\n", QueueIdx + 1 );
		Timer.mark();
		for( BcU32 JobIdx = 0; JobIdx < NoofJobs; ++JobIdx )
		{
			Kernel.pushFunctionJob( JobQueues[ QueueIdx ], TestIncJob );
			Kernel.pushFunctionJob( JobQueues[ QueueIdx ], TestDecJob );
		}
		Kernel.flushAllJobQueues();
		ThisTime = Timer.time() * 1000.0f;
		TotalTime += ThisTime;
		BcPrintf( "Time: %fms, (%fms total)\n", ThisTime, TotalTime );
		BcUnitTestMsg( IncDecAtomic.load( std::memory_order_seq_cst ) == 0, "Job queue w/ workers test." );
	}

	// Test all queues simultaneously.
	BcPrintf( "Begin: queues all\n" );
	Timer.mark();
	for( BcU32 QueueIdx = 0; QueueIdx < 4; ++QueueIdx )
	{
		for( BcU32 JobIdx = 0; JobIdx < NoofJobs; ++JobIdx )
		{
			Kernel.pushFunctionJob( JobQueues[ QueueIdx ], TestIncJob );
			Kernel.pushFunctionJob( JobQueues[ QueueIdx ], TestDecJob );
		}
	}
	Kernel.flushAllJobQueues();
	ThisTime = Timer.time() * 1000.0f;
	TotalTime += ThisTime;
	BcPrintf( "Time: %fms, (%fms total)\n", ThisTime, TotalTime );
	BcUnitTestMsg( IncDecAtomic.load( std::memory_order_seq_cst ) == 0, "All job queues." );
}


//////////////////////////////////////////////////////////////////////////
// Worker masks.
size_t SysKernel::DEFAULT_JOB_QUEUE_ID = (size_t)-1;

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

#if !PLATFORM_HTML5
	// Create job workers for the number of threads we have.
	BcU32 NoofThreads = BcMax( std::thread::hardware_concurrency(), 1 );

	// Increment fence by number of threads.
	JobWorkerStartFence_.increment( NoofThreads );
	
	JobWorkers_.reserve( NoofThreads );
	for( BcU32 Idx = 0; Idx < NoofThreads; ++Idx )
	{
		JobWorkers_.push_back( new SysJobWorker( this, JobWorkerStartFence_ ) );
	}

	// Wait for workers to start.
	JobWorkerStartFence_.wait( 0 );

	// Mark main timer.
	MainTimer_.mark();
#endif // !PLATFORM_HTML5
}

//////////////////////////////////////////////////////////////////////////
// Dtor
SysKernel::~SysKernel()
{
	// Stop.
	stop();

	// Join.
	if( ExecutionThread_.joinable() )
	{
		ExecutionThread_.join();
	}

#if !PLATFORM_HTML5	
	// Free job workers.
	for( auto JobWorker : JobWorkers_ )
	{
		delete JobWorker;
	}
	JobWorkers_.clear();
#endif // !PLATFORM_HTML5

	// Free job queues.
	for( auto JobQueue : JobQueues_ )
	{
		delete JobQueue;
	}
	JobQueues_.clear();
}

//////////////////////////////////////////////////////////////////////////
// createJobQueue
size_t SysKernel::createJobQueue( size_t NoofWorkers, size_t MinimumHardwareThreads )
{
	BcAssertMsg( BcIsGameThread(), "Should only create job queues on the game thread." );

#if !PLATFORM_HTML5
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

	size_t JobQueueId = JobQueues_.size();
	auto JobQueue = new SysJobQueue( this );
	JobQueues_.push_back( JobQueue );

	SysFence Fence( NoofWorkers );

	// Add new job queue to the workers.
	for( size_t Idx = 0; Idx < NoofWorkers; ++Idx )
	{
		size_t RealIdx = ( CurrWorkerAllocIdx_ + Idx ) % JobWorkers_.size();
		auto JobWorker( JobWorkers_[ RealIdx ] );
		JobWorker->addJobQueue( JobQueue );

		// Push a job for synchronisation.
		pushFunctionJob( JobQueueId, [ &Fence ]()
		{
			// Decrement, and then wait until every job has been executed.
			// This will ensure that each worker will be blocked until the next
			// takes the next job.
			Fence.decrement();
			Fence.wait( 0 );
		} );
	}

	// Flush so that we exit this function with the job queue ready to go.
	JobQueue->flushJobs( BcFalse );

	// Reassign worker allocation index.
	CurrWorkerAllocIdx_ = ( CurrWorkerAllocIdx_ + NoofWorkers ) % JobWorkers_.size(); 

	return JobQueueId;
#else
	return (size_t)-1;
#endif
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
size_t SysKernel::workerCount() const
{
	return JobWorkers_.size();
}

//////////////////////////////////////////////////////////////////////////
// addSystems
BcBool SysKernel::pushJob( size_t JobQueueId, SysJob* pJob )
{
#if !PLATFORM_HTML5
	// Check if we're out of range.
	if( JobQueueId < JobQueues_.size() )
	{
		return JobQueues_[ JobQueueId ]->pushJob( pJob );
	}
#endif

	// No queue we can use, execute on this thread.
	pJob->internalExecute();
	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// flushJobQueue
void SysKernel::flushJobQueue( size_t JobQueueId )
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
// getGameTheadTime
BcF32 SysKernel::getGameThreadTime() const
{
	return GameThreadTime_;
}

//////////////////////////////////////////////////////////////////////////
// runOnce
void SysKernel::runOnce()
{
#if PSY_USE_PROFILER
	static BcU32 FrameCount = 0;
#endif

#if PSY_USE_PROFILER
	if( FrameCount == 0 )
	{
		BcProfiler::pImpl()->beginProfiling();
	}
#endif

	// Store frame time.
	FrameTime_ = BcMin( (BcF32)MainTimer_.time(), TickRate_ * 4.0f );

	BcAssert( FrameTime_ >= 0.0f );

	// Mark main timer.
	MainTimer_.mark();
	
	// Tick systems.
	tick();

	// Store game thread time.
	GameThreadTime_ = (BcF32)MainTimer_.time();
	

#if !PLATFORM_HTML5
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
#endif
	

#if PSY_USE_PROFILER
	++FrameCount;

	if( FrameCount == 60 )
	{
		BcProfiler::pImpl()->endProfiling();
		FrameCount = 0;
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// execute
//virtual
void SysKernel::execute()
{
	// Set main thread.
	BcSetGameThread();
	
	// Run until there are no more systems to run.
	do
	{
		runOnce();
	}
	while( SystemList_.size() > 0 );
}

//////////////////////////////////////////////////////////////////////////
// notifySchedule
void SysKernel::notifySchedule()
{
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
