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

#include "Base/BcTypes.h"

#include <atomic>

////////////////////////////////////////////////////////////////////////////////
// SysFence
class SysFence
{
public:
	SysFence( size_t InitialValue = 0 );
	~SysFence();

	/**
	 * Increment fence.
	 */
	void increment( size_t Value = 1 );

	/**
	 * Decrement fence.
	 */
	void decrement( size_t Value = 1 );

	/*
	 * Get the count.
	 */
	size_t count() const;

	/**
	 * Wait for fence to reach a certain value.
	 */
	void wait( size_t Value = 0 ) const;

private:
	std::atomic< size_t > Count_;

};


#endif
