/**************************************************************************
*
* File:		SysFence.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/11	
* Description:
*		Fence for synchronisation of jobs.
*		
*
*
* 
**************************************************************************/

#ifndef __SYSFENCE_H__
#define __SYSFENCE_H__

#include "BcAtomic.h"

////////////////////////////////////////////////////////////////////////////////
// SysFence
class SysFence
{
public:
	SysFence();
	~SysFence();

	/**
	 * Increment fence.
	 */
	void increment();

	/**
	 * Decrement fence.
	 */
	void decrement();

	/**
	 * Is fence complete?
	 */
	BcBool isComplete();

	/**
	 * Queue up a fence job.
	 */
	void queue( BcU32 WorkerMask );

	/**
	 * Wait for fence.
	 */
	void wait();

private:
	BcAtomic< BcU32 > Count_;

};


#endif
