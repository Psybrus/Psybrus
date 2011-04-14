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

#include <deque>

//////////////////////////////////////////////////////////////////////////
// BcCommand
class BcCommand
{
public:
	virtual ~BcCommand(){};
	virtual void execute() = 0;
};

//////////////////////////////////////////////////////////////////////////
// BcCommandBuffer
class BcCommandBuffer
{
public:
	BcCommandBuffer();
	~BcCommandBuffer();
	
	/**
	 * Push a command into the queue.
	 */
	void push( BcCommand* pCommand );
	
	/**
	 * Execute commands in the queue.
	 */
	void execute();
	
private:
	BcMutex							QueueLock_;
	std::deque< BcCommand* >		CommandQueue_;
};

#endif