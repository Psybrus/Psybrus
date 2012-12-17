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
	CsCore::pImpl()->registerResource< AkBankComponent >();
	CsCore::pImpl()->registerResource< AkGameObjectComponent >();
	CsCore::pImpl()->registerResource< AkListenerComponent >();

	// Register + start Ak system core.
	if( GPsySetupParams.Flags_ & psySF_SOUND )
	{
		SYS_REGISTER( "AkCore", AkCore );
		SysKernel::pImpl()->startSystem( "AkCore" );
	}
}

void IntegrationWwise_Unregister()
{
	// Unregister resources.
	CsCore::pImpl()->unregisterResource< AkBank >();
	CsCore::pImpl()->unregisterResource< AkBankComponent >();
	CsCore::pImpl()->unregisterResource< AkGameObjectComponent >();
	CsCore::pImpl()->unregisterResource< AkListenerComponent >();
}
