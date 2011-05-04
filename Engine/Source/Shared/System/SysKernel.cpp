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

#ifdef PLATFORM_WINDOWS
#include "BcWindows.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Command line
BcChar** SysArgv_ = NULL;
BcU32 SysArgc_ = 0;

//////////////////////////////////////////////////////////////////////////
// Ctor
SysKernel::SysKernel()
{
	ShuttingDown_ = BcFalse;
	SleepAccumulator_ = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
SysKernel::~SysKernel()
{
	
}

//////////////////////////////////////////////////////////////////////////
// registerSystem
void SysKernel::registerSystem( const std::string& Name, SysSystemCreator creator )
{
	// Add to creator map.
	SystemCreatorMap_[ Name ] = creator;
}

//////////////////////////////////////////////////////////////////////////
// startSystem
SysSystem* SysKernel::startSystem( const std::string& Name )
{
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
	}
	return pSystem;
}

//////////////////////////////////////////////////////////////////////////
// stop
void SysKernel::stop()
{
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
void SysKernel::run()
{
	// Run until there are no more systems to run.
	do
	{
		// Mark main timer.
		MainTimer_.mark();
		
		// Tick systems.
		tick();
		
		// Grab time spent, and sleep the remainder of 1/60.
#if PSY_SERVER
		BcReal TickTime = 1.0f / 15.0f;
#else
		BcReal TickTime = 1.0f / 60.0f;
#endif
		BcReal TimeSpent = MainTimer_.time();
		SleepAccumulator_ += BcMax( ( TickTime ) - TimeSpent, 0.0f );
		
		if( SleepAccumulator_ > 0.0f )
		{
#ifdef PLATFORM_OSX
			//BcPrintf( "Time: %f ms, Slept: %f ms\n", TimeSpent * 1000.0f, SleepAccumulator_ * 1000.0f );

			// Platform specific hack, FIX ME LATER.
			BcU32 USleepTime = BcU32( SleepAccumulator_ * 1000000.0f );
			SleepAccumulator_ -= BcReal( USleepTime ) / 1000000.0f;
			::usleep( USleepTime );
#endif

#ifdef PLATFORM_WINDOWS
			//BcPrintf( "Time: %f ms, Slept: %f ms\n", TimeSpent * 1000.0f, SleepAccumulator_ * 1000.0f );

			// Platform specific hack, FIX ME LATER.
			BcU32 USleepTime = BcU32( SleepAccumulator_ * 1000.0f );
			SleepAccumulator_ -= BcReal( USleepTime ) / 1000.0f;
			::Sleep( USleepTime );
#endif
		}
	}
	while( SystemList_.size() > 0 );
}

//////////////////////////////////////////////////////////////////////////
// tick
void SysKernel::tick()
{
	if( ShuttingDown_ == BcFalse )
	{
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
		
		// Delete system.
		delete pRemSystem;
		
		// Advance to next system for removal.
		++RemIter;
	}
	
	PendingRemoveSystemList_.clear();
}
