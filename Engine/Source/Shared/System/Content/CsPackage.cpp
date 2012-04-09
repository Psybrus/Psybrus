/**************************************************************************
*
* File:		CsPackage.cpp
* Author:	Neil Richardson
* Ver/Date:	8/04/12
* Description:
*
*
*
*
*
**************************************************************************/

#include "System/Content/CsPackage.h"
#include "System/Content/CsPackageLoader.h"

#include "System/Content/CsCore.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPackage::CsPackage( const BcName& Name ):
	Name_( Name ),
	pLoader_( NULL )
{
	pLoader_ = new CsPackageLoader( this, CsCore::pImpl()->getPackagePackedPath( Name ) );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPackage::~CsPackage()
{
	delete pLoader_;
	pLoader_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// isReady
BcBool CsPackage::isReady() const
{
	for( BcU32 Idx = 0; Idx < Resources_.size(); ++Idx )
	{
		const CsResourceRef<>& Resource( Resources_[ Idx ] );

		// Check that the package is the only referencer.
		if( Resource.isReady() == BcFalse )
		{
			return BcFalse;
		}
	}

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// hasUnreferencedResources
BcBool CsPackage::hasUnreferencedResources() const
{
	BcAssert( BcIsGameThread() );

	// If the data isn't ready, we are still referenced.
	if( pLoader_->isDataReady() == BcFalse )
	{
		return BcFalse;
	}

	BcBool IsUnreferenced = BcTrue;
	
	for( BcU32 Idx = 0; Idx < Resources_.size(); ++Idx )
	{
		const CsResourceRef<>& Resource( Resources_[ Idx ] );

		// If we've got invalid resources, or a ref count of 1 (self-ref'd) we have unreferenced resources.
		if( Resource.isValid() == BcFalse || Resource.refCount() == 1 )
		{
			return BcTrue;
		}
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// releaseUnreferencedResources
void CsPackage::releaseUnreferencedResources()
{
	BcAssert( BcIsGameThread() );

	// If the data isn't ready, we are still referenced.
	if( pLoader_->isDataReady() == BcFalse )
	{
		return;
	}
	
	for( BcU32 Idx = 0; Idx < Resources_.size(); ++Idx )
	{
		CsResourceRef<>& Resource( Resources_[ Idx ] );

		// Check that the package is the only referencer, if so, NULL it.
		if( Resource.isValid() && Resource.refCount() == 1 )
		{
			Resource = NULL;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// addResource
void CsPackage::addResource( CsResource* pResource )
{
	Resources_.push_back( pResource );
}

//////////////////////////////////////////////////////////////////////////
// getResource
CsResource* CsPackage::getResource( BcU32 ResourceIdx )
{
	CsResource* pResource = NULL;
	if( ResourceIdx < Resources_.size() )
	{
		pResource = Resources_[ ResourceIdx ];
		BcAssert( pResource->getIndex() == ResourceIdx );
	}
	
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// getName
const BcName& CsPackage::getName() const
{
	return Name_;
}

//////////////////////////////////////////////////////////////////////////
// getName
const BcChar* CsPackage::getString( BcU32 Offset )
{
	return pLoader_->getString( Offset );
}

//////////////////////////////////////////////////////////////////////////
// getChunkSize
BcU32 CsPackage::getChunkSize( BcU32 ResourceIdx, BcU32 ResourceChunkIdx )
{
	return pLoader_->getChunkSize( ResourceIdx, ResourceChunkIdx );
}

//////////////////////////////////////////////////////////////////////////
// getNoofChunks
BcU32 CsPackage::getNoofChunks( BcU32 ResourceIdx )
{
	return pLoader_->getNoofChunks( ResourceIdx );
}

//////////////////////////////////////////////////////////////////////////
// requestChunk
BcBool CsPackage::requestChunk( BcU32 ResourceIdx, BcU32 ResourceChunkIdx, void* pDataLocation )
{
	return pLoader_->requestChunk( ResourceIdx, ResourceChunkIdx, pDataLocation );
}
