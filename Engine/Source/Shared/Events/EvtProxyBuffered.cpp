/**************************************************************************
*
* File:		EvtProxyBuffered.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event proxy buffered. Buffers events for controlled dispatch.
*		
*		
*
* 
**************************************************************************/

#include "EvtProxyBuffered.h"

#include "Base/BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
EvtProxyBuffered::EvtProxyBuffered( EvtPublisher* pPublisher, BcSize BufferSize ):
	EvtProxy( pPublisher )
{
	BufferSize_ = BufferSize;
	BufferPosition_ = 0;
	pBuffer_ = BcMemAlign( BufferSize, 16 );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
EvtProxyBuffered::~EvtProxyBuffered()
{
	BcMemFree( pBuffer_ );
	pBuffer_ = NULL;
	BufferSize_ = 0;
	BufferPosition_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// proxy
//virtual
void EvtProxyBuffered::proxy( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize )
{
	BcSize StorageRequirements = sizeof( EvtID ) + sizeof( BcSize ) + EventSize;
	BcSize NewBufferPosition = BufferPosition_ + StorageRequirements;

	BcAssertMsg( NewBufferPosition_ < BufferSize_, "EvtProxyBuffered: Event fills our buffer. Larger event buffer required! Dispatching immediately, expect logic to be broken!" );

	if( NewBufferPosition_ < BufferSize_ )
	{
		BcU8* pWriteBuffer = reinterpret_cast< BcU8* >( pBuffer ) + BufferPosition_;
	}
	else
	{
		EvtProxy::proxy( ID, EventBase, EventSize );
	}
}
