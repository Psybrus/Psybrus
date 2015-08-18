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

#include "System/File/FsCore.h"

//////////////////////////////////////////////////////////////////////////
// Define CsResource
REFLECTION_DEFINE_DERIVED( CsPackage );

//////////////////////////////////////////////////////////////////////////
// Reflection
void CsPackage::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "RefCount_",			&CsPackage::RefCount_,			bcRFF_TRANSIENT ),
		new ReField( "pLoader_",			&CsPackage::pLoader_,			bcRFF_TRANSIENT ),
		new ReField( "Resources_",			&CsPackage::Resources_,			bcRFF_TRANSIENT ),
	};
		
	ReRegisterClass< CsPackage, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPackage::CsPackage()
{
	// TODO: Path not implemented just yet.
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPackage::CsPackage( const BcName& Name, const BcPath& Filename ):
	RefCount_( 0 ),
	pLoader_( nullptr )
{
	setName( Name );

	// Keep attempting load.
	BcBool LoaderValid = BcFalse;
	do
	{
		BcBool PackedPackageExists = FsCore::pImpl()->fileExists( Filename.c_str() );

		if( PackedPackageExists )
		{
			pLoader_ = new CsPackageLoader( this, Filename );
	
			// If the loader has no error it's valid to continue.
			if( !pLoader_->hasError() )
			{
				LoaderValid = BcTrue;
			}
			else
			{
				delete pLoader_;
				pLoader_ = nullptr;
			}
		}

		// If loader has failed, prompt.
		if( LoaderValid == BcFalse )
		{
#if !PSY_PRODUCTION
			BcChar Buffer[ 4096 ] = { 0 };
			BcSPrintf( Buffer, sizeof( Buffer ) - 1, "Package \"%s\" is missing. Run importer and try again.", (*Name).c_str() );
			if( BcMessageBox( "Package error", Buffer, bcMBT_OKCANCEL, bcMBI_ERROR ) == bcMBR_OK )
			{
				BcSleep( 5.0f );
			}
			else
			{
				exit( 1 );
			}
#else
			// Exit.
			exit( 1 );
#endif
		}
	}
	while( LoaderValid == BcFalse );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPackage::~CsPackage()
{
	BcAssert( RefCount_ == 0 );
	BcAssert( pLoader_ == nullptr );
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
		const ReObjectRef< CsResource >& Resource( Resources_[ Idx ] );

		// Check that the package is the only referencer.
		if( ( Resource.isValid() && Resource->isReady() ) == BcFalse )
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
	if( pLoader_ != nullptr && pLoader_->isDataReady() == BcFalse )
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
	if( pLoader_ != nullptr && pLoader_->isDataReady() == BcFalse )
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
		const ReObjectRef< CsResource >& Resource( Resources_[ Idx ] );

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
	if( pLoader_ != nullptr && pLoader_->isDataReady() == BcFalse )
	{
		return;
	}
	
	for( BcU32 Idx = 0; Idx < Resources_.size(); ++Idx )
	{
		ReObjectRef< CsResource >& Resource( Resources_[ Idx ] );

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
	CsResource* pResource = nullptr;
	if( ResourceIdx < Resources_.size() )
	{
		pResource = Resources_[ ResourceIdx ];
		BcAssert( pResource->getIndex() == ResourceIdx );
	}
	
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// getCrossRefPackage
CsPackage* CsPackage::getCrossRefPackage( BcU32 ID )
{
	return pLoader_->getCrossRefPackage( ID );
}

//////////////////////////////////////////////////////////////////////////
// getCrossRefResource
CsResource* CsPackage::getCrossRefResource( BcU32 ID )
{
	return pLoader_->getCrossRefResource( ID );
}

//////////////////////////////////////////////////////////////////////////
// acquire
void CsPackage::acquire()
{
	++RefCount_;
}

//////////////////////////////////////////////////////////////////////////
// release
void CsPackage::release()
{
	auto RefCount = --RefCount_;	
	if( RefCount == 0 )
	{
		delete pLoader_;
		pLoader_ = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
// getString
const BcChar* CsPackage::getString( BcU32 Offset ) const
{
	return pLoader_->getString( Offset );
}

//////////////////////////////////////////////////////////////////////////
// getChunkSize
BcU32 CsPackage::getChunkSize( BcU32 ResourceIdx, BcU32 ResourceChunkIdx ) const
{
	return pLoader_->getChunkSize( ResourceIdx, ResourceChunkIdx );
}

//////////////////////////////////////////////////////////////////////////
// getNoofChunks
BcU32 CsPackage::getNoofChunks( BcU32 ResourceIdx ) const
{
	return pLoader_->getNoofChunks( ResourceIdx );
}

//////////////////////////////////////////////////////////////////////////
// markupName
void CsPackage::markupName( BcName& Name ) const
{
	Name = BcName( getString( Name.getID() ) );
}

//////////////////////////////////////////////////////////////////////////
// requestChunk
BcBool CsPackage::requestChunk( BcU32 ResourceIdx, BcU32 ResourceChunkIdx, void* pDataLocation )
{
	return pLoader_->requestChunk( ResourceIdx, ResourceChunkIdx, pDataLocation );
}
