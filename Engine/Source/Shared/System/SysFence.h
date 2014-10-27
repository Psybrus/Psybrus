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
	SysFence( BcU32 InitialValue = 0 );
	~SysFence();

	/**
	 * Increment fence.
	 */
	void increment( BcU32 Value = 1 );

	/**
	 * Decrement fence.
	 */
	void decrement( BcU32 Value = 1 );

	/*
	 * Get the count.
	 */
	BcU32 count() const;

	/**
	 * Wait for fence to reach a certain value.
	 */
	void wait( BcU32 Value = 0 ) const;

private:
	std::atomic< BcU32 > Count_;

};


#endif
