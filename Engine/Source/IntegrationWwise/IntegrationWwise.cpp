/**************************************************************************
*
* File:		IntegrationWwise.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Integration of Audiokinetic Wwise for Psybrus.
*		
*
*
* 
**************************************************************************/

#include "IntegrationWwise.h"

#include "System/Audiokinetic/AkBank.h"
#include "System/Audiokinetic/AkGameObject.h"
#include "System/Audiokinetic/AkListener.h"
#include "System/Audiokinetic/AkRTPC.h"

#include <windows.h>

namespace AK
{
	void * AllocHook( size_t in_size )
	{
		return BcMemAlign( in_size );
	}

	void FreeHook( void * in_ptr )
	{
		BcMemFree( in_ptr );
	}

	void * VirtualAllocHook(
		void * in_pMemAddress,
		size_t in_size,
		DWORD in_dwAllocationType,
		DWORD in_dwProtect
		)
	{
		return ::VirtualAlloc( in_pMemAddress, in_size, in_dwAllocationType, in_dwProtect );
	}

	void VirtualFreeHook( 
		void * in_pMemAddress,
		size_t in_size,
		DWORD in_dwFreeType
		)
	{
		::VirtualFree( in_pMemAddress, in_size, in_dwFreeType );
	}
}


void IntegrationWwise_Register()
{
	// Register resources.
	CsCore::pImpl()->registerResource< AkBank >();
	CsCore::pImpl()->registerResource< AkGameObjectComponent >();
	CsCore::pImpl()->registerResource< AkListenerComponent >();
	CsCore::pImpl()->registerResource< AkRTPCComponent >();

	// Register + start Ak system core.
	SYS_REGISTER( "AkCore", AkCore );
	SysKernel::pImpl()->startSystem( "AkCore" );
}

void IntegrationWwise_Unregister()
{
	// Unregister resources.
	CsCore::pImpl()->unregisterResource< AkBank >();
	CsCore::pImpl()->unregisterResource< AkGameObjectComponent >();
	CsCore::pImpl()->unregisterResource< AkListenerComponent >();
	CsCore::pImpl()->unregisterResource< AkRTPCComponent >();
}
