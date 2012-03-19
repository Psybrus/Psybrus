/**************************************************************************
*
* File:		CsPackage.h
* Author:	Neil Richardson
* Ver/Date:	19/03/12
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __CSPACKAGE_H__
#define __CSPACKAGE_H__

#include "CsResource.h"

//////////////////////////////////////////////////////////////////////////
// CsPackage
class CsPackage
{
private:
	CsPackage( const CsPackage& ){}

public:
	CsPackage( const BcName& PackageName );
	~CsPackage();

	/**
	 * Get resource out of package.
	 */
	template< typename _Ty >
	BcBool				getResource( const BcName& Name, CsResourceRef< _Ty >& Handle );

private:
	BcBool				internalGetResource( const BcName& Name, const BcName& Type, CsResourceRef<>& Handle );
	
private:
	typedef std::pair< BcName, BcName > TResourceNameTypePair;
	typedef std::map< TResourceNameTypePair, CsResourceRef<> > TResourceMap;
	typedef TResourceMap::iterator TResourceMapIterator;

	TResourceMap		ResourceMap_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
template< typename _Ty >
BcForceInline BcBool CsPackage::getResource( const BcName& Name, CsResourceRef< _Ty >& Handle )
{
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	return internalGetResource( Name, _Ty::StaticGetType(), InternalHandle );
}


#endif
