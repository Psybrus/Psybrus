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
void BcCommandBuffer::push( BcCommand* pCommand )
{
	QueueLock_.lock();
	CommandQueue_.push_back( pCommand );
	QueueLock_.unlock();
}

//////////////////////////////////////////////////////////////////////////
// execute
void BcCommandBuffer::execute()
{
	// Queue of commands to execute.
	std::deque< BcCommand* > CommandExecuteQueue;
	
	// Copy and clear the queue so we can execute.
	QueueLock_.lock();
	CommandExecuteQueue = CommandQueue_;
	CommandQueue_.clear();
	QueueLock_.unlock();
	
	// Execute all commands in the queue.
	while( CommandExecuteQueue.size() > 0 )
	{
		// Grab command.
		BcCommand* pCommand = CommandExecuteQueue.front();
		CommandExecuteQueue.pop_front();
		
		// Execute command.
		pCommand->execute();
		
		// Delete command.
		delete pCommand;
	}	
}
