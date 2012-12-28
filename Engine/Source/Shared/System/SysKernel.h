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
#include "Base/BcThread.h"
#include "System/SysSystem.h"
#include "System/SysJobQueue.h"
#include "System/SysDelegateDispatcher.h"

#include <list>
#include <map>
#include <string>

//////////////////////////////////////////////////////////////////////////
// Command line params
extern std::string SysArgs_;

//////////////////////////////////////////////////////////////////////////
// SysKernel
class SysKernel:
	public BcGlobal< SysKernel >,
	public BcThread
{
public:
	static BcU32 SYSTEM_WORKER_MASK;
	static BcU32 USER_WORKER_MASK;

public:
	SysKernel( BcF32 TickRate );
	~SysKernel();
	
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
	void						enqueueJob( BcU32 WorkerMask, SysJob* pJob );

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
	 * Enqueue job.
	 */
	template< typename _Fn, typename _P0, typename _P1, typename _P2, typename _P3 >
	BcForceInline void			enqueueDelegateJob( BcU32 WorkerMask, const BcDelegate< _Fn >& Delegate, _P0 P0, _P1 P1, _P2 P2, _P3 P3 )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall( P0, P1, P2, P3 );
		enqueueJob( WorkerMask, new SysDelegateJob( pDelegateCall ) );		
	}

	/**
	 * Enqueue job.
	 */
	template< typename _Fn, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4 >
	BcForceInline void			enqueueDelegateJob( BcU32 WorkerMask, const BcDelegate< _Fn >& Delegate, _P0 P0, _P1 P1, _P2 P2, _P3 P3, _P4 P4 )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall( P0, P1, P2, P3, P4 );
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
	
	/**
	 * Enqueue callback.
	 */
	template< typename _Fn, typename _P0, typename _P1, typename _P2, typename _P3 >
	BcForceInline void enqueueCallback( const BcDelegate< _Fn >& Delegate, _P0 P0, _P1 P1, _P2 P2, _P3 P3 )
	{
		BcDelegateCall< _Fn >* pDelegateCall = new BcDelegateCall< _Fn >( Delegate );
		pDelegateCall->deferCall( P0, P1, P2, P3 );
		DelegateDispatcher_.enqueueDelegateCall( pDelegateCall );
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
	
	TSystemCreatorMap			SystemCreatorMap_;
	TSystemList					PendingAddSystemList_;
	TSystemList					PendingRemoveSystemList_;
	TSystemList					SystemList_;
	BcBool						ShuttingDown_;
	
	BcMutex						SystemLock_;
	BcBool						IsThreaded_;
	
	BcTimer						MainTimer_;
	
	BcF32						SleepAccumulator_;
	BcF32						TickRate_;
	BcF32						FrameTime_;
	BcF32						GameThreadTime_;
	
	SysJobQueue					JobQueue_;
	SysDelegateDispatcher		DelegateDispatcher_;
};

#endif

