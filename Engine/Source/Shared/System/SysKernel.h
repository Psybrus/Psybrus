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
#include "BcTimer.h"
#include "SysSystem.h"
#include "SysJobQueue.h"
#include "SysDelegateDispatcher.h"

#include <list>
#include <map>
#include <string>

//////////////////////////////////////////////////////////////////////////
// Command line params
extern BcChar** SysArgv_;

extern BcU32 SysArgc_;
//////////////////////////////////////////////////////////////////////////
// SysKernel
class SysKernel:
	public BcGlobal< SysKernel >
{
public:
	SysKernel();
	~SysKernel();
	
	/**
	 * Register system.
	 */
	void						registerSystem( const std::string& Name, SysSystemCreator creator );
	
	/**
	 * Start system.
	 */
	SysSystem*					startSystem( const std::string& Name );
	
	/**
	 * Stop kernel.
	 */
	void						stop();
		
	/**
	 * Run kernel.
	 */
	void						run();
	
	/**
	 * Tick kernel.
	 */
	void						tick();
	
	/**
	 * Queue job.
	 */
	void						queueJob( SysJob* pJob, BcU32 WorkerMask = 0xffffffff );

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

private:
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
	typedef std::map< std::string, SysSystemCreator > TSystemCreatorMap;
	typedef TSystemCreatorMap::iterator TSystemCreatorMapIterator;
	
	TSystemCreatorMap			SystemCreatorMap_;
	TSystemList					PendingAddSystemList_;
	TSystemList					PendingRemoveSystemList_;
	TSystemList					SystemList_;
	BcBool						ShuttingDown_;
	
	BcTimer						MainTimer_;
	
	BcReal						SleepAccumulator_;
	
	SysJobQueue					JobQueue_;
	SysDelegateDispatcher		DelegateDispatcher_;
};

#endif

