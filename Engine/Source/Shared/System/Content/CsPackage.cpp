/**************************************************************************
*
* File:		CsPackage.cpp
* Author:	Neil Richardson
* Ver/Date:	19/03/12
* Description:
*
*
*
*
*
**************************************************************************/

#include "CsPackage.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPackage::CsPackage( const BcName& PackageName )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPackage::~CsPackage()
{

}

//////////////////////////////////////////////////////////////////////////
// internalGetResource
BcBool CsPackage::internalGetResource( const BcName& Name, const BcName& Type, CsResourceRef<>& Handle )
{
	TResourceNameTypePair KeyPair( Name, Type );
	TResourceMapIterator It( ResourceMap_.find( KeyPair) );

	if( It != ResourceMap_.end() )
	{
		Handle = (*It).second;
		return BcTrue;
	}

	return BcFalse;
}
