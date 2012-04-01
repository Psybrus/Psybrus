/**************************************************************************
*
* File:		BcThread.h
* Author: 	Neil Richardson 
* Ver/Date:	0.2 - 31/01/06
*			0.3 - 10/12/06
* Description:
*		A C++ Class Thread Implementation.
*		
*		
* 
**************************************************************************/

#ifndef __BCTHREAD_H__
#define __BCTHREAD_H__

#include "BcTypes.h"

#include <pthread.h>

// And now the cThread class, which can be inherited
// to give an object its own thread. Will be handy for
// any manager that can be shoved onto another thread.
class BcThread
{
public:

	/**
	 * Just make sure everything is nice and NULLified.
	 */
	BcThread();
	
	/**
	 * This cReally should do something, like make sure the thread has stopped...
	 */
	virtual ~BcThread();

	/**
	 * This is used to start the thread to handle the object that
	 * has inherited this object.
	 */
	BcU32 start( const BcChar* pName = "Unnamed" );

	/**
	 * This will 'join' this thread with the calling one.
	 * Effectively waiting for it to quit.
	 */
	BcU32 join();

	/**
	 * Is thread active?
	 */
	BcBool isActive();

protected:
	virtual void execute()=0;

private:
	static void* threadMain( void* pParam );

	BcBool			IsActive_;
	pthread_t		ThreadHandle_;

};

#endif
