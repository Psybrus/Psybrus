/**************************************************************************
*
* File:		CsFileReaderRPC.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Resource file
*		
*
*
* 
**************************************************************************/

#include "CsFileReaderRPC.h"
#include "CsCoreClient.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
CsFileReaderRPC::CsFileReaderRPC( const std::string& Name ):
	CsFile( Name )
{
	BcMemZero( &Header_, sizeof( Header_ ) );
	pChunks_ = NULL;
	pData_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsFileReaderRPC::~CsFileReaderRPC()
{
	delete [] pChunks_;
	delete [] pData_;
}

//////////////////////////////////////////////////////////////////////////
// load
BcBool CsFileReaderRPC::load( CsFileReadyDelegate ReadyDelegate, CsFileChunkDelegate ChunkDelegate )
{
	// Store delegates.
	ReadyDelegate_ = ReadyDelegate;
	ChunkDelegate_ = ChunkDelegate;
	
	// Do RPC via core.
	CsCore::pImpl< CsCoreClient >()->delegateLoad( this, ReadyDelegate, ChunkDelegate );
	
	// Impossible to know the real success at this point.
	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// getChunk
const CsFileChunk* CsFileReaderRPC::getChunk( BcU32 Chunk, BcBool TriggerLoad )
{
	CsFileChunk* pChunk = &pChunks_[ Chunk ];
	CsFileChunkProps* pChunkProps = &pChunkProps_[ Chunk ];
	
	// Do the RPC if we want to trigger a load.
	if( TriggerLoad == BcTrue )
	{
		// Check if not loaded, and load if need be.
		if( pChunkProps->Status_.compareExchange( CsFileChunkProps::STATUS_LOADING, CsFileChunkProps::STATUS_NOT_LOADED ) == CsFileChunkProps::STATUS_NOT_LOADED )
		{
			// Do RPC via core.
			CsCore::pImpl< CsCoreClient >()->delegateGetChunk( this, Chunk );
		}
	}
	
	return pChunk;
}

//////////////////////////////////////////////////////////////////////////
// getID
BcU32 CsFileReaderRPC::getID() const
{
	return Header_.ID_;
}

//////////////////////////////////////////////////////////////////////////
// getNoofChunks
BcU32 CsFileReaderRPC::getNoofChunks() const
{
	return Header_.NoofChunks_;
}

//////////////////////////////////////////////////////////////////////////
// onHeaderLoaded
void CsFileReaderRPC::onHeaderLoaded( void* pData, BcSize Size )
{
	// Check we have the right data.
	BcAssert( Size == sizeof( Header_ ) );

	// Copy in the data.
	BcMemCopy( &Header_, pData, Size );
	
	// Loaded header, now allocate the correct number of chunks & props
	pChunks_ = new CsFileChunk[ Header_.NoofChunks_ ];
	pChunkProps_ = new CsFileChunkProps[ Header_.NoofChunks_ ];
	
	// Clear chunk props.
	for( BcU32 i = 0; i < Header_.NoofChunks_; ++i )
	{
		pChunkProps_[ i ].Status_ = CsFileChunkProps::STATUS_NOT_LOADED;	
	}
}

//////////////////////////////////////////////////////////////////////////
// onChunksLoaded
void CsFileReaderRPC::onChunksLoaded( void* pData, BcSize Size )
{
	// Check we have the right data.
	BcAssert( Size == sizeof( CsFileChunk ) * Header_.NoofChunks_ );

	// Copy in the data.
	BcMemCopy( pChunks_, pData, Size );
	
	// Calculate total size.
	TotalDataSize_ = 0;
	for( BcU32 i = 0; i < Header_.NoofChunks_; ++i )
	{
		TotalDataSize_ += pChunks_[ i ].Size_;			
	}
	
	// Allocate data block.
	pData_ = new BcU8[ TotalDataSize_ ];
	
	// Call delegate.
	ReadyDelegate_( this );
}

//////////////////////////////////////////////////////////////////////////
// onDataLoaded
void CsFileReaderRPC::onDataLoaded( void* pData, BcSize Size )
{
	// Chunk index is fed in as we can't simply match pointers here.
	BcU32 ChunkIdx = *(BcU32*)pData;
	
	pData = ((BcU32*)pData) + 1;
	Size -= sizeof( BcU32 );
	
	// Find the chunk that data matches to.
	if( ChunkIdx < Header_.NoofChunks_ )
	{
		CsFileChunk* pChunk = &pChunks_[ ChunkIdx ];
		CsFileChunkProps* pChunkProps = &pChunkProps_[ ChunkIdx ];
		
		BcAssert( pChunk->Size_ == Size );
	
		// Copy data and cache pointer.
		BcU32 TotalHeaderSize = sizeof( CsFileHeader ) + sizeof( CsFileChunk ) * Header_.NoofChunks_;
		void* pInternalData = pData_ + ( pChunk->Offset_ - TotalHeaderSize );
		BcMemCopy( pInternalData, pData, Size );

		BcAssert( (BcU8*)pInternalData < ( pData_ + TotalDataSize_ ) );
		
		pChunkProps->Status_ = CsFileChunkProps::STATUS_LOADED;
		ChunkDelegate_( this, ChunkIdx, pChunk, pInternalData );
	}
}



