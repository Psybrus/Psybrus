/**************************************************************************
*
* File:		GaPackage.cpp
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaPackage.h"
#include "CsCore.h"
#include "BcString.h"

#ifdef PSY_SERVER
#include "BcStream.h"
#endif

#ifdef PSY_SERVER

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool GaPackage::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	const Json::Value& ImportResources = Object[ "resources" ];
	
	// Import all resources for this package.
	std::vector< CsResourceRef<> > Resources;
	CsResourceRef<> ResourceRef;
	for( BcU32 Idx = 0; Idx < ImportResources.size(); ++Idx )
	{
		const Json::Value& Resource = ImportResources[ Idx ];
		
		if( CsCore::pImpl()->importObject( Resource, ResourceRef, DependancyList ) )
		{
			Resources.push_back( ResourceRef );
		}
	}
	
	// Export.
	if( Resources.size() > 0 )
	{
		BcStream HeaderStream;
		BcStream ResourceStream;
		
		THeader Header = 
		{
			Resources.size()
		};
		
		HeaderStream << Header;
		
		for( BcU32 Idx = 0; Idx < Resources.size(); ++Idx )
		{
			TResourceHeader ResourceHeader;
			
			BcStrCopyN( ResourceHeader.Type_, Resources[ Idx ]->getTypeString().c_str(), sizeof( ResourceHeader.Type_ ) );
			BcStrCopyN( ResourceHeader.Name_, Resources[ Idx ]->getName().c_str(), sizeof( ResourceHeader.Name_ ) );
			
			ResourceStream << ResourceHeader;
		}
		
		pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
		pFile_->addChunk( BcHash( "resources" ), ResourceStream.pData(), ResourceStream.dataSize() );
		
		return BcTrue;
	}
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaPackage );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaPackage::initialise()
{
	pHeader_ = NULL;
	pResourceHeaders_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void GaPackage::create()
{
	
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void GaPackage::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool GaPackage::isReady()
{
	// TODO: Thread safety!
	for( BcU32 Idx = 0; Idx < ResourceRefList_.size(); ++Idx )
	{
		if( ResourceRefList_[ Idx ]->isReady() == BcFalse )
		{
			return BcFalse;
		}
	}
	
	return pHeader_ != NULL && pResourceHeaders_ != NULL && ResourceRefList_.size() == pHeader_->NoofResources_;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void GaPackage::fileReady()
{
	// File is ready, get the header chunk.
	getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void GaPackage::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		pHeader_ = (THeader*)pData;
		
		getChunk( ++ChunkIdx );
	}
	else if( pChunk->ID_ == BcHash( "resources" ) )
	{
		pResourceHeaders_ = (TResourceHeader*)pData;
		
		ResourceRefList_.reserve( pHeader_->NoofResources_ );
		
		for( BcU32 Idx = 0; Idx < pHeader_->NoofResources_; ++Idx )
		{
			CsResourceRef<> Handle;
			TResourceHeader* pResourceHeader = &pResourceHeaders_[ Idx ];
			
			if( CsCore::pImpl()->internalRequestResource( pResourceHeader->Name_, pResourceHeader->Type_, Handle ) )
			{
				ResourceRefList_.push_back( Handle );
			}
		}
	}
}

