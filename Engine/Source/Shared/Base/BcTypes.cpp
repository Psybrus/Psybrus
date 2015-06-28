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

namespace CompilerUtility
{
	std::string Demangle( const char* Name ) 
	{
		int Status = 1;

		std::unique_ptr< char, void(*)(void*) > Res
		{
			abi::__cxa_demangle( Name, nullptr, nullptr, &Status ),
			std::free
		};

		return ( Status == 0 ) ? Res.get() : Name;
	}
}

#else

namespace CompilerUtility
{
	std::string Demangle( const char* Name )
	{
	    return Name;
	}
}

#endif // ( COMPILER_GCC || COMPILER_CLANG ) && !PLATFORM_ANDROID
