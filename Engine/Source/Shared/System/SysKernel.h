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

#include "BcGlobal.h"
#include "BcName.h"
#include "BcTimer.h"
#include "BcThread.h"
#include "SysSystem.h"
#include "SysJobQueue.h"
#include "SysDelegateDispatcher.h"

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
	SysKernel( BcReal TickRate );
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
	BcReal						getFrameTime() const;

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
	
	BcReal						SleepAccumulator_;
	BcReal						TickRate_;
	BcReal						FrameTime_;
	
	SysJobQueue					JobQueue_;
	SysDelegateDispatcher		DelegateDispatcher_;
};

#endif

