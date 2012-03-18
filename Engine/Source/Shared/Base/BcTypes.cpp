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
