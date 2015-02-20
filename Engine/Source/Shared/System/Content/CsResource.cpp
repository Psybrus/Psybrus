/**************************************************************************
*
* File:		CsResource.cpp
* Author:	Neil Richardson 
* Ver/Date:	7/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Content/CsResource.h"
 
#include "System/Content/CsCore.h"
#include "System/Content/CsPackage.h"

//////////////////////////////////////////////////////////////////////////
// Define CsResource
REFLECTION_DEFINE_DERIVED( CsResource );

//////////////////////////////////////////////////////////////////////////
// Reflection
void CsResource::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Index_", &CsResource::Index_, bcRFF_TRANSIENT ),
		new ReField( "InitStage_", &CsResource::InitStage_, bcRFF_TRANSIENT ),
	};
		
	ReRegisterClass< CsResource, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsResource::CsResource( ReNoInit ):
	Index_( BcErrorCode ),
	InitStage_( INIT_STAGE_INITIAL )
{
	CsCore::pImpl()->internalAddResource( this );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsResource::CsResource():
	Index_( BcErrorCode ),
	InitStage_( INIT_STAGE_INITIAL )
{
	CsCore::pImpl()->internalAddResource( this );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
CsResource::~CsResource()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void CsResource::initialise()
{
	
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void CsResource::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void CsResource::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void CsResource::fileReady()
{

}

//////////////////////////////////////////////////////////////////////////
// setIndex
void CsResource::setIndex(  BcU32 Index )
{
	Index_ = Index;
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool CsResource::isReady() const
{
	BcAssertMsg( InitStage_ >= INIT_STAGE_INITIAL && InitStage_ <= INIT_STAGE_DESTROY, "CsResource: Invalid ready state." );
	return InitStage_ == INIT_STAGE_READY ? BcTrue : BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// getInitStage
BcU32 CsResource::getInitStage() const
{
	return InitStage_;
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void CsResource::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// getName
CsPackage* CsResource::getPackage() const
{
	CsPackage* RetVal = nullptr;
	if( getRootOwner() != nullptr )
	{
		BcAssert( getOwner()->isTypeOf< CsPackage >() );
		RetVal = static_cast< CsPackage* >( getRootOwner() );
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// getName
const BcName& CsResource::getPackageName() const
{
	return getPackage() != nullptr ? getPackage()->getName() : BcName::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// getIndex
BcU32 CsResource::getIndex() const
{
	return Index_;
}

//////////////////////////////////////////////////////////////////////////
// getString
const BcChar* CsResource::getString( BcU32 Offset ) const
{
	return getPackage()->getString( Offset );
}

//////////////////////////////////////////////////////////////////////////
// markupName
void CsResource::markupName( BcName& Name ) const
{
	getPackage()->markupName( Name );
}

//////////////////////////////////////////////////////////////////////////
// getChunk
void CsResource::requestChunk( BcU32 Chunk, void* pDataLocation )
{
	if( Index_ != BcErrorCode )
	{
		if( !getPackage()->requestChunk( Index_, Chunk, pDataLocation ) )
		{
	
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getChunkSize
BcU32 CsResource::getChunkSize( BcU32 Chunk )
{
	if( Index_ != BcErrorCode )
	{
		return getPackage()->getChunkSize( Index_, Chunk );
	}
	else
	{
		PSY_LOG( "WARNING: Attempting to get chunk size where we have an invalid index. Resource: %s\n", (*getName()).c_str() );
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// getNoofChunks
BcU32 CsResource::getNoofChunks() const
{
	if( Index_ != BcErrorCode )
	{
		return getPackage()->getNoofChunks( Index_ );
	}
	else
	{
		PSY_LOG( "WARNING: Attempting to get number of chunks where we have an invalid index. Resource: %s\n", (*getName()).c_str() );
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// markCreate
void CsResource::markCreate()
{
	BcU32 OldStage = InitStage_.exchange( INIT_STAGE_CREATE );
	BcAssertMsg( OldStage == INIT_STAGE_INITIAL, "CsResource: Trying to mark \"%s\" for creation when it's not in the initial state.", (*getName()).c_str() );
	BcUnusedVar( OldStage );
	create();
}

//////////////////////////////////////////////////////////////////////////
// markReady
void CsResource::markReady()
{
	BcU32 OldStage = InitStage_.exchange( INIT_STAGE_READY );
	BcAssertMsg( OldStage == INIT_STAGE_CREATE, "CsResource: Trying to mark \"%s\" as ready when it's not in creation.", (*getName()).c_str() );
	BcUnusedVar( OldStage );
}

//////////////////////////////////////////////////////////////////////////
// markReady
void CsResource::markDestroy()
{
	BcU32 OldStage = InitStage_.exchange( INIT_STAGE_DESTROY );
	BcAssertMsg( OldStage == INIT_STAGE_READY, "CsResource: Trying to mark \"%s\" for destruction when it's not ready.", (*getName()).c_str() );
	BcUnusedVar( OldStage );
	destroy();
	CsCore::pImpl()->internalAddResourceForProcessing( this );
}

//////////////////////////////////////////////////////////////////////////
// onFileReady
void CsResource::onFileReady()
{
	fileReady();
}

//////////////////////////////////////////////////////////////////////////
// onFileChunkReady
void CsResource::onFileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	fileChunkReady( ChunkIdx, ChunkID, pData );
}
