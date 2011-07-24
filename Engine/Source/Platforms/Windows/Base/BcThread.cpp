/**************************************************************************
*
* File:		BcThread.cpp
* Author: 	Neil Richardson 
* Ver/Date:	0.2 - 31/01/06
*			0.3 - 10/12/06
*			0.4 - 27/02/07
* Description:
*		A C++ Class Thread Implementation.
*		
*		
* 
**************************************************************************/

#include "BcThread.h"
#include "BcDebug.h"

BcThread::BcThread():
	m_ThreadID( 0 ),
	m_ThreadHandle( NULL ),
	m_isThreadActive( BcFalse ),
	bSuspended_( BcFalse )
{

}

BcThread::~BcThread()
{
	join();
}

BcU32 BcThread::start()
{
	DWORD CreationFlags = THREAD_PRIORITY_NORMAL;

	/* TODO: Implement again later.
	switch ( Priority )
	{
	case tp_LOW:
		CreationFlags |= THREAD_PRIORITY_BELOW_NORMAL;
		break;
	case tp_NORMAL:
		CreationFlags |= THREAD_PRIORITY_NORMAL;
		break;
	case tp_HIGH:
		CreationFlags |= THREAD_PRIORITY_ABOVE_NORMAL;
		break;
	};
	*/

	m_isThreadActive = BcTrue;
	m_ThreadHandle = ::CreateThread( 0, 0, BcThread::entryPoint, static_cast< void* >( this ), 0, &m_ThreadID );

	return 0;
}

BcU32 BcThread::join()
{
	if( m_ThreadHandle != NULL )
	{
		::WaitForSingleObject( m_ThreadHandle, INFINITE );
		BcAssert( m_isThreadActive == BcFalse );
		::CloseHandle( m_ThreadHandle );
	}
	return 0;
}

BcBool BcThread::isActive()
{
	return m_isThreadActive;
}

DWORD WINAPI BcThread::entryPoint( LPVOID l_pThis )
{
	BcThread* l_pCastThis = reinterpret_cast< BcThread* >( l_pThis );

	// Now call users overloaded function...
	l_pCastThis->execute();
	
	// Oh, its quit has it? Lets make sure the thread is no longer active then!
	l_pCastThis->m_isThreadActive = BcFalse;
	
	return 0;
}
