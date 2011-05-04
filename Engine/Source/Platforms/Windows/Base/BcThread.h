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

#include <windows.h>

enum eThreadPriority
{
	tp_LOW = 0,
	tp_NORMAL,
	tp_HIGH
};

// And now the cThread class, which can be inherited
// to give an object its own thread. Will be handy for
// any manager that can be shoved onto another thread.
class BcThread
{
public:

	// Function: BcThread()
	// Just make sure everything is nice and NULLified.
	BcThread();
	
	// Function: ~BcThread()
	// This cReally should do something, like make sure the thread has stopped...
	virtual ~BcThread();

	// Function: BcU32 start()
	// This is used to start the thread to handle the object that
	// has inherited this object.
	BcU32 start( eThreadPriority Priority = tp_NORMAL );

	// Function: BcU32 join()
	// This will 'join' this thread with the calling one.
	// Effectively waiting for it to quit.
	BcU32 join();

	// Function: BcU32 resume()
	// Will resume the thread if suspended.
	BcU32 resume();

	// Function: BcU32 suspend()
	// Will suspend the thread.
	BcU32 suspend();

	// Function: BcBool isActive()
	// Will return cTrue if the thread is active.
	BcBool isActive();

	// Function: void waitOnSuspend()
	// Will wait until thread is suspended. Shouldn't be called inside the thread.
	void waitOnSuspend();

	// Function: void yield()
	// Will release some cpu time to other threads.
	void yield();

protected:

	virtual void execute()=0;

	static DWORD WINAPI entryPoint( LPVOID l_pThis );

private:

	DWORD	m_ThreadID;
	HANDLE	m_ThreadHandle;

	volatile BcBool m_isThreadActive;
	volatile BcBool bSuspended_;
};

#endif
