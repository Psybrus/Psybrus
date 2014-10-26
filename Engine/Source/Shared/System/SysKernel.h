/**************************************************************************
*
* File:		SysCore.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		System kernel
*		
*
*
* 
**************************************************************************/

#ifndef __SYSKERNEL_H__
#define __SYSKERNEL_H__

#include "Base/BcGlobal.h"
#include "Base/BcName.h"
#include "Base/BcTimer.h"
#include "System/SysSystem.h"
#include "System/SysJob.h"
#include "System/SysDelegateDispatcher.h"

#include "Reflection/ReReflection.h"

#include <thread>
#include <list>
#include <map>
#include <string>
#include <condition_variable>

//////////////////////////////////////////////////////////////////////////
// Command line params
extern std::string SysArgs_;

//////////////////////////////////////////////////////////////////////////
// SysKernel
class SysKernel:
	public BcGlobal< SysKernel >
{
public:
	REFLECTION_DECLARE_BASE_MANUAL_NOINIT( SysKernel );

public:
	static size_t DEFAULT_JOB_QUEUE_ID;

public:
	SysKernel( ReNoInit );
	SysKernel( BcF32 TickRate = 1.0f / 60.0f );
	~SysKernel();

	/**
	 * Create job queue.
	 * Not thread safe. Should only call from the game thread during initialisation.
	 * Job queues are destroyed when the engine shuts down.
	 * @param NoofWorkers Number of workers to create.
	 * @param MinimumHardwareThreads Minimum number of hardware threads required to create a worker.
	 * @return ID of job queue.
	 */
	size_t createJobQueue( size_t NoofWorkers, size_t MinimumHardwareThreads );
	
	/**
	 * Register system.
	 */
	void registerSystem( const BcName& Name, SysSystemCreator creator );
	
	/**
	 * Start system.
	 */
	SysSystem* startSystem( const BcName& Name );
	
	/**
	 * Stop kernel.
	 */
	void stop();
	
	/**
	 * Run kernel.
	 * @param Threaded Do we want to run the kernel threaded?
	 */
	void run( BcBool Threaded );
	
	/**
	 * Tick kernel.
	 */
	void tick();

	/**
	 * Get worker count.
	 */
	size_t workerCount() const;
	
	/**
	 * Push job.
	 * @param JobQueueId ID of job queue to use.
	 */
	BcBool pushJob( size_t JobQueueId, class SysJob* pJob );

	/**
	 * Flush all jobs in the queue.
	 * @param JobQueueId Job queue ID.
	 */
	void flushJobQueue( size_t JobQueueId );

	/**
	 * Flush all job queues.
	 */
	void flushAllJobQueues();

	/**
	 * Get frame time.
	 */
	BcF32 getFrameTime() const;

	/**
	 * Enqueue job.
	 */
	template< typename _Fn >
	BcForceInline void			pushDelegateJob( size_t JobQueueId, const BcDelegate< _Fn >& Delegate )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall();
		BcBool RetVal = pushJob( JobQueueId, new SysDelegateJob( pDelegateCall ) );		
		BcAssert( RetVal );
	}

	/**
	 * Enqueue job.
	 */
	template< typename _Fn, typename _P0 >
	BcForceInline void			pushDelegateJob( size_t JobQueueId, const BcDelegate< _Fn >& Delegate, _P0 P0 )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall( P0 );
		BcBool RetVal = pushJob( JobQueueId, new SysDelegateJob( pDelegateCall ) );		
		BcAssert( RetVal );
	}

	/**
	 * Enqueue job.
	 */
	template< typename _Fn, typename _P0, typename _P1 >
	BcForceInline void			pushDelegateJob( size_t JobQueueId, const BcDelegate< _Fn >& Delegate, _P0 P0, _P1 P1 )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall( P0, P1 );
		BcBool RetVal = pushJob( JobQueueId, new SysDelegateJob( pDelegateCall ) );		
		BcAssert( RetVal );
	}	
	
	/**
	 * Enqueue job.
	 */
	template< typename _Fn, typename _P0, typename _P1, typename _P2 >
	BcForceInline void			pushDelegateJob( size_t JobQueueId, const BcDelegate< _Fn >& Delegate, _P0 P0, _P1 P1, _P2 P2 )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall( P0, P1, P2 );
		BcBool RetVal = pushJob( JobQueueId, new SysDelegateJob( pDelegateCall ) );		
		BcAssert( RetVal );
	}

	/**
	 * Push function job.
	 */
	BcForceInline void pushFunctionJob( size_t JobQueueId, std::function< void() > Function )
	{
		BcBool RetVal = pushJob( JobQueueId, new SysFunctionJob( Function ) );
		BcAssert( RetVal );
	}

	/**
	 * Enqueue callback.
	 */
	template< typename _Fn >
	BcForceInline void enqueueCallback( const BcDelegate< _Fn >& Delegate )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall();
		DelegateDispatcher_.enqueueDelegateCall( pDelegateCall );
	}
	
	/**
	 * Enqueue callback.
	 */
	template< typename _Fn, typename _P0 >
	BcForceInline void enqueueCallback( const BcDelegate< _Fn >& Delegate, _P0 P0 )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall( P0 );
		DelegateDispatcher_.enqueueDelegateCall( pDelegateCall );
	}

	/**
	 * Enqueue callback.
	 */
	template< typename _Fn, typename _P0, typename _P1 >
	BcForceInline void enqueueCallback( const BcDelegate< _Fn >& Delegate, _P0 P0, _P1 P1 )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall( P0, P1 );
		DelegateDispatcher_.enqueueDelegateCall( pDelegateCall );
	}
	
	/**
	 * Enqueue callback.
	 */
	template< typename _Fn, typename _P0, typename _P1, typename _P2 >
	BcForceInline void enqueueCallback( const BcDelegate< _Fn >& Delegate, _P0 P0, _P1 P1, _P2 P2 )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall( P0, P1, P2 );
		DelegateDispatcher_.enqueueDelegateCall( pDelegateCall );
	}

private:
	friend class SysJobWorker;
	friend class SysJobQueue;

	/**
	 * Wait for schedule.
	 */
	template < class _Predicate >
	inline void waitForSchedule( std::mutex& Mutex, _Predicate Pred )
	{
		// NOTE: condition variable used here seems to be a problem.
		//       Need to read up on correct usage, as it seems
		//       to work perfectly in Windows, and not in Linux.
#if PLATFORM_LINUX
		bool WaitPred = !Pred();
		while( WaitPred )
		{
			BcSleep( 0.001f );
			Mutex.lock();
			WaitPred = !Pred();
			Mutex.unlock();
		}
#else
		std::unique_lock< std::mutex > Lock( Mutex );
		JobQueued_.wait( Lock, Pred );
#endif
	}

	/**
	 * Notify for scheduling.
	 */
	void notifySchedule();
	
private:
	/**
	 * Execute.
	 */ 
	virtual void execute();
	
	/**
	* Add systems.
	*/
	void addSystems();
	
	/**
	* Remove systems.
	*/
	void removeSystems();
	
private:
	typedef std::list< SysSystem* > TSystemList;
	typedef TSystemList::iterator TSystemListIterator;
	typedef TSystemList::reverse_iterator TSystemListReverseIterator;
	typedef std::map< BcName, SysSystemCreator > TSystemCreatorMap;
	typedef TSystemCreatorMap::iterator TSystemCreatorMapIterator;

	std::thread ExecutionThread_;
	
	TSystemCreatorMap SystemCreatorMap_;
	TSystemList PendingAddSystemList_;
	TSystemList PendingRemoveSystemList_;
	TSystemList SystemList_;
	BcBool ShuttingDown_;
	
	std::recursive_mutex SystemLock_;
	BcBool IsThreaded_;
	
	BcTimer MainTimer_;

	BcF32 SleepAccumulator_;
	BcF32 TickRate_;
	BcF32 FrameTime_;
	BcF32 GameThreadTime_;
	
	std::vector< class SysJobQueue* > JobQueues_;
	std::vector< class SysJobWorker* > JobWorkers_;
	size_t CurrWorkerAllocIdx_;

	std::condition_variable JobQueued_;
	SysDelegateDispatcher DelegateDispatcher_;
};

#endif

