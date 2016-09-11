/**************************************************************************
*
* File:		BcTypes.cpp
* Author:	Neil Richardson 
* Ver/Date:	25/09/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "Base/BcTypes.h"

#include "Base/BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// BcTypes_UnitTest
void BcTypes_UnitTest()
{
	// TODO: Move this into a types unit test.
	BcUnitTest( sizeof( BcU8 ) == 1 );
	BcUnitTest( sizeof( BcS8 ) == 1 );
	BcUnitTest( sizeof( BcU16 ) == 2 );
	BcUnitTest( sizeof( BcS16 ) == 2 );
	BcUnitTest( sizeof( BcU32 ) == 4 );
	BcUnitTest( sizeof( BcS32 ) == 4 );
	BcUnitTest( sizeof( BcU64 ) == 8 );
	BcUnitTest( sizeof( BcS64 ) == 8 );
	BcUnitTest( sizeof( BcF32 ) == 4 );
	BcUnitTest( sizeof( BcF64 ) == 8 );
}

//////////////////////////////////////////////////////////////////////////
// TODO: Move this to a better place...
// Mega super awesome hack.
// Temporary until better demangling is setup.
#if ( COMPILER_GCC || COMPILER_CLANG )
#include <cxxabi.h>
#include <stdlib.h>

namespace CompilerUtility
{
	bool Demangle( const char* Name, char* Output, size_t OutputSize )
	{
		int Status = 1;

		std::unique_ptr< char, void(*)(void*) > Res
		{
			abi::__cxa_demangle( Name, nullptr, nullptr, &Status ),
			free
		};

		if( Status == 0 )
		{
			BcStrCopy( Output, OutputSize, Res.get() );
			return true;
		}
		return false;
	}
}

#else

namespace CompilerUtility
{
	bool Demangle( const char* Name, char* Output, size_t OutputSize )
	{
		BcStrCopy( Output, (BcU32)OutputSize, Name );
	    return true;
	}
}

#endif // ( COMPILER_GCC || COMPILER_CLANG ) && !PLATFORM_ANDROID
