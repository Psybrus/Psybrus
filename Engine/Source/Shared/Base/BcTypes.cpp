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

#include "BcTypes.h"

#include "BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// BcTypes_UnitTest
void BcTypes_UnitTest()
{
	// TODO: Move this into a types unit test.
	BcAssert( sizeof( BcU8 ) == 1 );
	BcAssert( sizeof( BcS8 ) == 1 );
	BcAssert( sizeof( BcU16 ) == 2 );
	BcAssert( sizeof( BcS16 ) == 2 );
	BcAssert( sizeof( BcU32 ) == 4 );
	BcAssert( sizeof( BcS32 ) == 4 );
	BcAssert( sizeof( BcU64 ) == 8 );
	BcAssert( sizeof( BcS64 ) == 8 );
	BcAssert( sizeof( BcF32 ) == 4 );
	BcAssert( sizeof( BcF64 ) == 8 );
}
