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

#include "Base/BcAtomic.h"

////////////////////////////////////////////////////////////////////////////////
// SysFence
class SysFence
{
public:
	SysFence();
	SysFence( BcU32 WorkerMask );  // Immediately queue and wait for 0.
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
	 * Queue up a fence job.
	 */
	void queue( BcU32 WorkerMask );

	/*
	 * Get the count.
	 */
	BcU32 count() const;

	/**
	 * Wait for fence to reach a certain value.
	 */
	void wait( BcU32 Value = 0 );

private:
	BcAtomic< BcU32 > Count_;

};


#endif
