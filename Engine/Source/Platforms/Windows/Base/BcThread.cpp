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

const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

static void SetThreadName( DWORD dwThreadID, const char* threadName )
{
	if( ::IsDebuggerPresent() )
	{
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = threadName;
		info.dwThreadID = dwThreadID;
		info.dwFlags = 0;
	
		__try
		{
			RaiseException( MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info );
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
}

BcThread::BcThread():
	m_ThreadID( 0 ),
	m_ThreadHandle( NULL ),
	m_isThreadActive( BcFalse )
{

}

BcThread::~BcThread()
{
	join();
}

BcU32 BcThread::start( const BcChar* pName )
{
	DWORD CreationFlags = THREAD_PRIORITY_NORMAL;

	m_isThreadActive = BcTrue;
	m_ThreadHandle = ::CreateThread( 0, 0, BcThread::entryPoint, static_cast< void* >( this ), 0, &m_ThreadID );

	if( pName != NULL )
	{
		::SetThreadName( m_ThreadID, pName );
	}

	return 0;
}

BcU32 BcThread::join()
{
	if( m_ThreadHandle != NULL )
	{
		::WaitForSingleObject( m_ThreadHandle, INFINITE );
		BcAssert( m_isThreadActive == BcFalse );
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
