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
	// Register + start Ak system core.
	SYS_REGISTER( "AksCore", AksCore );
	SysKernel::pImpl()->startSystem( "AksCore" );
}

void IntegrationWwise_Unregister()
{

}
