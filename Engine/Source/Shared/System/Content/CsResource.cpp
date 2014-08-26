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
CsResource::CsResource():
	Index_( BcErrorCode ),
	InitStage_( INIT_STAGE_INITIAL )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
CsResource::~CsResource()
{
	
}

//////////////////////////////////////////////////////////////////////////
// preInitialise
void CsResource::preInitialise( const BcName& Name, BcU32 Index, CsPackage* pPackage )
{
	BcAssertMsg( Name != BcName::INVALID, "Resource can not have an invalid name." );
	BcAssertMsg( Name != BcName::NONE, "Resource can not have a none name." );

	setName( Name );
	setOwner( pPackage );
	Index_ = Index;
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
// isReady
//virtual
BcBool CsResource::isReady() const
{
	BcAssertMsg( InitStage_ >= INIT_STAGE_INITIAL && InitStage_ <= INIT_STAGE_READY, "CsResource: Invalid ready state." );
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
		BcPrintf( "WARNING: Attempting to get chunk size where we have an invalid index. Resource: %s\n", (*getName()).c_str() );
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
		BcPrintf( "WARNING: Attempting to get number of chunks where we have an invalid index. Resource: %s\n", (*getName()).c_str() );
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// markCreate
void CsResource::markCreate()
{
	BcU32 OldStage = InitStage_.exchange( INIT_STAGE_CREATE );
	BcAssertMsg( OldStage == INIT_STAGE_INITIAL, "CsResource: Trying to mark \"%s\" for creation when it's not in the initial state.", (*getName()).c_str() );
}

//////////////////////////////////////////////////////////////////////////
// markReady
void CsResource::markReady()
{
	BcU32 OldStage = InitStage_.exchange( INIT_STAGE_READY );
	BcAssertMsg( OldStage == INIT_STAGE_CREATE, "CsResource: Trying to mark \"%s\" as ready when it's not in creation.", (*getName()).c_str() );
}

//////////////////////////////////////////////////////////////////////////
// markReady
void CsResource::markDestroy()
{
	BcU32 OldStage = InitStage_.exchange( INIT_STAGE_DESTROY );
	BcAssertMsg( OldStage == INIT_STAGE_READY, "CsResource: Trying to mark \"%s\" for destruction when it's not ready.", (*getName()).c_str() );

	CsCore::pImpl()->destroyResource( this );
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
