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
#include "System/Content/CsPackageImporter.h"

#include "System/Content/CsCore.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPackage::CsPackage( const BcName& Name ):
	Name_( Name ),
	pLoader_( NULL )
{
	// Cache paths related to this package.
	const BcPath PackedPackage( CsCore::pImpl()->getPackagePackedPath( Name ) );
	const BcPath ImportPackage( CsCore::pImpl()->getPackageImportPath( Name ) );

	// Keep attempting load.
	BcBool LoaderValid = BcFalse;
	do
	{
		BcBool PackedPackageExists = FsCore::pImpl()->fileExists( (*PackedPackage).c_str() );

		if( PackedPackageExists )
		{
			pLoader_ = new CsPackageLoader( this, PackedPackage );
	
			// If the loader has no error it's valid to continue.
			if( !pLoader_->hasError() )
			{
				LoaderValid = BcTrue;
			}
			else
			{
				delete pLoader_;
				pLoader_ = NULL;
			}
		}

#if PSY_SERVER
		// Attempt to import.
		if( LoaderValid == BcFalse )
		{
			BcBool ImportPackageExists = FsCore::pImpl()->fileExists( (*ImportPackage).c_str() );
			BcAssertMsg( ImportPackageExists, "CsPackage: Missing import package \"%s\"", (*ImportPackage).c_str() );

			if( ImportPackageExists )
			{
				CsPackageImporter Importer;
				BcBool ImportSucceeded = Importer.import( Name );
				BcAssertMsg( ImportSucceeded, "CsPackage: Failed to import \"%s\"", (*ImportPackage).c_str() );
			}
		}
#endif
	}
	while( LoaderValid == BcFalse );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPackage::~CsPackage()
{
	// The loader is cleaned up when the ref count is 0 (so it frees quick), so we expect ref count to be 0, and loader to be NULL.
	BcAssert( RefCount_ == 0 );
	BcAssert( pLoader_ == NULL );
}

//////////////////////////////////////////////////////////////////////////
// isReady
BcBool CsPackage::isReady() const
{
	// If the data isn't ready, we aren't ready.
	if( pLoader_->isDataReady() == BcFalse )
	{
		return BcFalse;
	}

	// Check our resources.
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
// isLoaded
BcBool CsPackage::isLoaded() const
{
	return pLoader_->isDataLoaded();
}

//////////////////////////////////////////////////////////////////////////
// hasUnreferencedResources
BcBool CsPackage::hasUnreferencedResources() const
{
	BcAssert( BcIsGameThread() );

	if( RefCount_ != 0 )
	{
		return BcFalse;
	}

	// If the data isn't ready, we are still referenced.
	if( pLoader_ != NULL && pLoader_->isDataReady() == BcFalse )
	{
		return BcFalse;
	}

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// haveAnyValidResources
BcBool CsPackage::haveAnyValidResources() const
{
	BcAssert( BcIsGameThread() );

	// If the data isn't ready, we will have valid resources soon.
	if( pLoader_ != NULL && pLoader_->isDataReady() == BcFalse )
	{
		return BcTrue;
	}

	// If our resource list is empty we can exit early.
	if( Resources_.size() == 0 )
	{
		return BcFalse;
	}

	// Search through list for all valid resources.
	for( BcU32 Idx = 0; Idx < Resources_.size(); ++Idx )
	{
		const CsResourceRef<>& Resource( Resources_[ Idx ] );

		if( Resource.isValid() == BcTrue )
		{
			return BcTrue;
		}
	}

	//
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// releaseUnreferencedResources
void CsPackage::releaseUnreferencedResources()
{
	BcAssert( BcIsGameThread() );

	if( RefCount_ != 0 )
	{
		return;
	}

	// If the data isn't ready, we are still referenced.
	if( pLoader_ != NULL && pLoader_->isDataReady() == BcFalse )
	{
		return;
	}
	
	for( BcU32 Idx = 0; Idx < Resources_.size(); ++Idx )
	{
		CsResourceRef<>& Resource( Resources_[ Idx ] );

		Resource->markDestroy();
	}

	Resources_.clear();
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
// loadPackageCrossRef
CsPackage* CsPackage::loadPackageCrossRef( BcU32 ID )
{
	BcName PackageName;
	BcName ResourceName;
	BcName TypeName;
	BcBool IsWeak;
	pLoader_->getPackageCrossRef( ID, PackageName, ResourceName, TypeName, IsWeak );
	return CsCore::pImpl()->requestPackage( PackageName );
}

//////////////////////////////////////////////////////////////////////////
// getPackageCrossRef
CsResourceRef<> CsPackage::getPackageCrossRef( BcU32 ID )
{
	CsResourceRef<> Resource;
	CsPackage* pPackage = NULL;
	BcName PackageName;
	BcName ResourceName;
	BcName TypeName;
	BcBool IsWeak;
	pLoader_->getPackageCrossRef( ID, PackageName, ResourceName, TypeName, IsWeak );

	// If it's a weak reference, we only want to find...not request.
	if( IsWeak )
	{
		// Request package, and check it's ready
		pPackage = CsCore::pImpl()->findPackage( PackageName );

		// If it's not ready or not loaded, return a NULL resource. Up to the user to handle.
		if( pPackage == NULL ||
			pPackage->isReady() == BcFalse )
		{
			return NULL;
		}
	}
	else
	{
		// Request package, and check it's ready
		pPackage = CsCore::pImpl()->requestPackage( PackageName );
		BcAssertMsg( pPackage->isLoaded(), "CsPackage: Package \"%s\" is not loaded, \"%s\" needs it loaded.", (*PackageName).c_str(), (*Name_).c_str() );
	}

	// Find resource.
	// NOTE: Should get from package for faster lookup.
	CsCore::pImpl()->internalFindResource( PackageName, ResourceName, TypeName, Resource );

	// If there is no valid resource at this point, then we must fail.
	BcAssertMsg( Resource.isValid(), "CsPackage: Cross ref isn't valid!" );

	//
	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// acquire
void CsPackage::acquire()
{
	RefCount_++;
}

//////////////////////////////////////////////////////////////////////////
// release
void CsPackage::release()
{
	if( --RefCount_ == 0 )
	{
		delete pLoader_;
		pLoader_ = NULL;
	}
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
