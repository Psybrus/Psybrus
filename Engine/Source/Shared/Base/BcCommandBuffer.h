/**************************************************************************
 *
 * File:	BcCommandBuffer.h
 * Author: 	Neil Richardson
 * Ver/Date:	
 * Description:
 *		Non-performance critical command buffer.
 *		Designed for multiple producer, single consumer.
 *
 * 
 **************************************************************************/

#ifndef __BCCOMMANDBUFFER_H__
#define __BCCOMMANDBUFFER_H__

#include "BcMutex.h"
#include "BcDelegate.h"

#include <deque>

//////////////////////////////////////////////////////////////////////////
// BcCommandBuffer
class BcCommandBuffer
{
public:
	BcCommandBuffer();
	~BcCommandBuffer();
	
	/**
	 * Enqueue a delegate call.
	 */
	void enqueueDelegateCall( BcDelegateCallBase* pDelegateCall );

	/**
	 * Execute commands in the queue.
	 */
	void execute();

public:
	template< typename _Ty >
	BcForceInline void enqueue( _Ty& DelegateCall )
	{
		_Ty* pDelegateCall = new _Ty( DelegateCall.deferCall() );
		enqueueDelegateCall( pDelegateCall );
	}

	template< typename _Ty, typename _P0 >
	BcForceInline void enqueue( _Ty& DelegateCall, _P0 P0 )
	{
		_Ty* pDelegateCall = new _Ty( DelegateCall.deferCall( P0 ) );
		enqueueDelegateCall( pDelegateCall );
	}

	template< typename _Ty, typename _P0, typename _P1 >
	BcForceInline void enqueue( _Ty& DelegateCall, _P0 P0, _P1 P1 )
	{
		_Ty* pDelegateCall = new _Ty( DelegateCall.deferCall( P0, P1 ) );
		enqueueDelegateCall( pDelegateCall );
	}

	template< typename _Ty, typename _P0, typename _P1, typename _P2 >
	BcForceInline void enqueue( _Ty& DelegateCall, _P0 P0, _P1 P1, _P2 P2 )
	{
		_Ty* pDelegateCall = new _Ty( DelegateCall.deferCall( P0, P1, P2 ) );
		enqueueDelegateCall( pDelegateCall );
	}

	template< typename _Ty, typename _P0, typename _P1, typename _P2, typename _P3 >
	BcForceInline void enqueue( _Ty& DelegateCall, _P0 P0, _P1 P1, _P2 P2, _P3 P3 )
	{
		_Ty* pDelegateCall = new _Ty( DelegateCall.deferCall( P0, P1, P2, P3 ) );
		enqueueDelegateCall( pDelegateCall );
	}
	
	template< typename _Ty, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4 >
	BcForceInline void enqueue( _Ty& DelegateCall, _P0 P0, _P1 P1, _P2 P2, _P3 P3, _P4 P4 )
	{
		_Ty* pDelegateCall = new _Ty( DelegateCall.deferCall( P0, P1, P2, P3, P4 ) );
		enqueueDelegateCall( pDelegateCall );
	}

	template< typename _Ty, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5 >
	BcForceInline void enqueue( _Ty& DelegateCall, _P0 P0, _P1 P1, _P2 P2, _P3 P3, _P4 P4, _P5 P5 )
	{
		_Ty* pDelegateCall = new _Ty( DelegateCall.deferCall( P0, P1, P2, P3, P4, P5 ) );
		enqueueDelegateCall( pDelegateCall );
	}

	template< typename _Ty, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5, typename _P6 >
	BcForceInline void enqueue( _Ty& DelegateCall, _P0 P0, _P1 P1, _P2 P2, _P3 P3, _P4 P4, _P5 P5, _P6 P6 )
	{
		_Ty* pDelegateCall = new _Ty( DelegateCall.deferCall( P0, P1, P2, P3, P4, P5, P6 ) );
		enqueueDelegateCall( pDelegateCall );
	}

	template< typename _Ty, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5, typename _P6, typename _P7 >
	BcForceInline void enqueue( _Ty& DelegateCall, _P0 P0, _P1 P1, _P2 P2, _P3 P3, _P4 P4, _P5 P5, _P6 P6, _P7 P7 )
	{
		_Ty* pDelegateCall = new _Ty( DelegateCall.deferCall( P0, P1, P2, P3, P4, P5, P6, P7 ) );
		enqueueDelegateCall( pDelegateCall );
	}

private:
	BcMutex									QueueLock_;
	std::deque< BcDelegateCallBase* >		CommandQueue_;
	std::deque< BcDelegateCallBase* >		ExecuteCommandQueue_;
};

#endif