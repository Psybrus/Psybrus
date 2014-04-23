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
	static BcU32 SYSTEM_WORKER_MASK;
	static BcU32 USER_WORKER_MASK;

public:
	SysKernel( ReNoInit );
	SysKernel( BcF32 TickRate = 1.0f / 60.0f );
	~SysKernel();

	/**
	 * Create job queue.
	 * Not thread safe. Should only call from the game thread during initialisation.
	 * @param NoofWorkers Number of workers to create.
	 * @return ID of job queue.
	 */
	BcU32						createJobQueue( BcU32 NoofWorkers );
	
	/**
	 * Register system.
	 */
	void						registerSystem( const BcName& Name, SysSystemCreator creator );
	
	/**
	 * Start system.
	 */
	SysSystem*					startSystem( const BcName& Name );
	
	/**
	 * Stop kernel.
	 */
	void						stop();
	
	/**
	 * Run kernel.
	 * @param Threaded Do we want to run the kernel threaded?
	 */
	void						run( BcBool Threaded );
	
	/**
	 * Tick kernel.
	 */
	void						tick();

	/**
	 * Get worker count.
	 */
	BcU32						workerCount() const;
	
	/**
	 * Enqueue job.
	 */
	void						enqueueJob( BcU32 WorkerMask, class SysJob* pJob );

	/**
	 * Get frame time.
	 */
	BcF32						getFrameTime() const;

	/**
	 * Enqueue job.
	 */
	template< typename _Fn >
	BcForceInline void			enqueueDelegateJob( BcU32 WorkerMask, const BcDelegate< _Fn >& Delegate )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall();
		enqueueJob( WorkerMask, new SysDelegateJob( pDelegateCall ) );		
	}

	/**
	 * Enqueue job.
	 */
	template< typename _Fn, typename _P0 >
	BcForceInline void			enqueueDelegateJob( BcU32 WorkerMask, const BcDelegate< _Fn >& Delegate, _P0 P0 )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall( P0 );
		enqueueJob( WorkerMask, new SysDelegateJob( pDelegateCall ) );		
	}

	/**
	 * Enqueue job.
	 */
	template< typename _Fn, typename _P0, typename _P1 >
	BcForceInline void			enqueueDelegateJob( BcU32 WorkerMask, const BcDelegate< _Fn >& Delegate, _P0 P0, _P1 P1 )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall( P0, P1 );
		enqueueJob( WorkerMask, new SysDelegateJob( pDelegateCall ) );		
	}
	
	/**
	 * Enqueue job.
	 */
	template< typename _Fn, typename _P0, typename _P1, typename _P2 >
	BcForceInline void			enqueueDelegateJob( BcU32 WorkerMask, const BcDelegate< _Fn >& Delegate, _P0 P0, _P1 P1, _P2 P2 )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall( P0, P1, P2 );
		enqueueJob( WorkerMask, new SysDelegateJob( pDelegateCall ) );		
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

	/**
	 * Wait for schedule.
	 */
	template < class _Predicate >
	inline void waitForSchedule( _Predicate Pred )
	{
		std::unique_lock< std::mutex > Lock( JobQueuedMutex_ );
		JobQueued_.wait( Lock, Pred );
	}

	/**
	 * Notify schedule.
	 */
	inline void notifySchedule()
	{
		JobQueued_.notify_all();
	}
	
private:
	/**
	 * Execute.
	 */ 
	virtual void				execute();
	
	/**
	* Add systems.
	*/
	void						addSystems();
	
	/**
	* Remove systems.
	*/
	void						removeSystems();
	
private:
	typedef std::list< SysSystem* > TSystemList;
	typedef TSystemList::iterator TSystemListIterator;
	typedef TSystemList::reverse_iterator TSystemListReverseIterator;
	typedef std::map< BcName, SysSystemCreator > TSystemCreatorMap;
	typedef TSystemCreatorMap::iterator TSystemCreatorMapIterator;

	std::thread					ExecutionThread_;
	
	TSystemCreatorMap			SystemCreatorMap_;
	TSystemList					PendingAddSystemList_;
	TSystemList					PendingRemoveSystemList_;
	TSystemList					SystemList_;
	BcBool						ShuttingDown_;
	
	std::recursive_mutex		SystemLock_;
	BcBool						IsThreaded_;
	
	BcTimer						MainTimer_;
	
	BcF32						SleepAccumulator_;
	BcF32						TickRate_;
	BcF32						FrameTime_;
	BcF32						GameThreadTime_;
	
	std::vector< class SysJobQueue* >	JobQueues_;
	std::vector< class SysJobWorker* >	JobWorkers_;

	std::condition_variable		JobQueued_;
	std::mutex					JobQueuedMutex_;
	SysDelegateDispatcher		DelegateDispatcher_;
};

#endif

