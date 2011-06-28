/**************************************************************************
 *
 * File:	BcCommandBuffer.cpp
 * Author: 	Neil Richardson
 * Ver/Date:	
 * Description:
 *		Non-performance critical command buffer.
 *		Designed for multiple producer, single consumer.
 *
 *
 * 
 **************************************************************************/


#include "BcCommandBuffer.h"


//////////////////////////////////////////////////////////////////////////
// Ctor
BcCommandBuffer::BcCommandBuffer()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcCommandBuffer::~BcCommandBuffer()
{
	
}

//////////////////////////////////////////////////////////////////////////
// push
void BcCommandBuffer::enqueueDelegateCall( BcDelegateCallBase* pDelegateCall )
{
	QueueLock_.lock();
	CommandQueue_.push_back( pDelegateCall );
	QueueLock_.unlock();
}

//////////////////////////////////////////////////////////////////////////
// execute
void BcCommandBuffer::execute()
{
	// Copy and clear the queue so we can execute.
	QueueLock_.lock();
	ExecuteCommandQueue_= CommandQueue_;
	CommandQueue_.clear();
	QueueLock_.unlock();
	
	// Execute all commands in the queue.
	while( ExecuteCommandQueue_.size() > 0 )
	{
		// Grab command.
		BcDelegateCallBase* pCommand = ExecuteCommandQueue_.front();
		ExecuteCommandQueue_.pop_front();
		
		// Execute command.
		(*pCommand)();
		
		// Delete command.
		delete pCommand;
	}	
}
