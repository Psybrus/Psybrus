/**************************************************************************
*
* File:		CsFileReader.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Resource file
*		
*
*
* 
**************************************************************************/

#include "CsFileReader.h"

#include "SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
CsFileReader::CsFileReader( const std::string& Name ):
	CsFile( Name )
{
	BcMemZero( &Header_, sizeof( Header_ ) );
	pStringTable_ = NULL;
	pChunks_ = NULL;
	pData_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsFileReader::~CsFileReader()
{
	delete [] pStringTable_;
	delete [] pChunks_;
	delete [] pData_;
}

//////////////////////////////////////////////////////////////////////////
// load
BcBool CsFileReader::load( CsFileReadyDelegate ReadyDelegate, CsFileChunkDelegate ChunkDelegate )
{
	// Store delegate.
	ReadyDelegate_ = ReadyDelegate;
	ChunkDelegate_ = ChunkDelegate;
	
	// Open file and read the header in.
	if( File_.open( Name_.c_str(), fsFM_READ ) )
	{
		File_.readAsync( 0, &Header_, sizeof( Header_ ), FsFileOpDelegate::bind< CsFileReader, &CsFileReader::onHeaderLoaded >( this ) );
		
		return BcTrue;
	}
	
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// getString
const BcChar* CsFileReader::getString( BcU32 Offset )
{
	if( Offset < Header_.StringTableSize_ )
	{
		return &pStringTable_[ Offset ];
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getChunk
const CsFileChunk* CsFileReader::getChunk( BcU32 Chunk, BcBool TriggerLoad )
{
	CsFileChunk* pChunk = &pChunks_[ Chunk ];
	CsFileChunkProps* pChunkProps = &pChunkProps_[ Chunk ];

	if( TriggerLoad == BcTrue )
	{
		BcU32 TotalHeaderSize = sizeof( CsFileHeader ) + Header_.StringTableSize_ + sizeof( CsFileChunk ) * Header_.NoofChunks_;
		BcU8* pData = pData_ + ( pChunk->Offset_ - TotalHeaderSize );

		// Bind the file op delegate.
		FsFileOpDelegate FileOpDelegate = FsFileOpDelegate::bind< CsFileReader, &CsFileReader::onDataLoaded >( this );

		// Check if not loaded, and load if need be.
		if( pChunkProps->Status_.compareExchange( CsFileChunkProps::STATUS_LOADING, CsFileChunkProps::STATUS_NOT_LOADED ) == CsFileChunkProps::STATUS_NOT_LOADED )
		{
			File_.readAsync( pChunk->Offset_, pData, pChunk->Size_, FileOpDelegate );
		}
		else
		{
			// Dispatch the callback to be performed after this call.
			SysKernel::pImpl()->enqueueCallback( FileOpDelegate, pData, pChunk->Size_ );
		}
	}
	
	return pChunk;
}

//////////////////////////////////////////////////////////////////////////
// getID
BcU32 CsFileReader::getID() const
{
	return Header_.ID_;
}

//////////////////////////////////////////////////////////////////////////
// getNoofChunks
BcU32 CsFileReader::getNoofChunks() const
{
	return Header_.NoofChunks_;
}

//////////////////////////////////////////////////////////////////////////
// onHeaderLoaded
void CsFileReader::onHeaderLoaded( void* pData, BcSize Size )
{
	// Check we have the right data.
	BcAssert( pData == &Header_ );
	BcAssert( Size == sizeof( Header_ ) );
	
	// Loaded header, now allocate the string table, chunks & props.
	pStringTable_ = new BcChar[ Header_.StringTableSize_ ];
	pChunks_ = new CsFileChunk[ Header_.NoofChunks_ ];
	pChunkProps_ = new CsFileChunkProps[ Header_.NoofChunks_ ];

	// Clear string table.
	BcMemZero( pStringTable_, Header_.StringTableSize_ );
	
	// Clear chunk props.
	for( BcU32 i = 0; i < Header_.NoofChunks_; ++i )
	{
		pChunkProps_[ i ].Status_ = CsFileChunkProps::STATUS_NOT_LOADED;	
	}
	
	// Load the string table in.
	BcU32 Position = sizeof( Header_ );
	BcU32 Bytes = Header_.StringTableSize_;
	File_.readAsync( Position, pStringTable_, Bytes, FsFileOpDelegate::bind< CsFileReader, &CsFileReader::onStringTableLoaded >( this ) );
}

//////////////////////////////////////////////////////////////////////////
// onStringTableLoaded
void CsFileReader::onStringTableLoaded( void* pData, BcSize Size )
{
	// Check we have the right data.
	BcAssert( pData == pStringTable_ );
	BcAssert( Size == Header_.StringTableSize_ );
	
	// No need to do anything here.
	
	// Load the chunks in.
	BcU32 Position = sizeof( Header_ ) + Header_.StringTableSize_;
	BcU32 Bytes = sizeof( CsFileChunk ) * Header_.NoofChunks_;
	File_.readAsync( Position, pChunks_, Bytes, FsFileOpDelegate::bind< CsFileReader, &CsFileReader::onChunksLoaded >( this ) );
}

//////////////////////////////////////////////////////////////////////////
// onChunksLoaded
void CsFileReader::onChunksLoaded( void* pData, BcSize Size )
{
	// Check we have the right data.
	BcAssert( pData == pChunks_ );
	BcAssert( Size == sizeof( CsFileChunk ) * Header_.NoofChunks_ );
	
	// Calculate total size.
	BcU32 TotalSize = 0;
	for( BcU32 i = 0; i < Header_.NoofChunks_; ++i )
	{
		TotalSize += pChunks_[ i ].Size_;			
	}
	
	// Allocate data block.
	pData_ = new BcU8[ TotalSize ];
	
	// Call delegate.
	ReadyDelegate_( this );
}

//////////////////////////////////////////////////////////////////////////
// onDataLoaded
void CsFileReader::onDataLoaded( void* pData, BcSize Size )
{
	//
	BcUnusedVar( Size );
	
	// Find the chunk that data matches to.
	BcU32 TotalHeaderSize = sizeof( CsFileHeader ) + Header_.StringTableSize_ + sizeof( CsFileChunk ) * Header_.NoofChunks_;
	BcU32 ChunkIdx = BcErrorCode;
	CsFileChunk* pFoundChunk = NULL;
	CsFileChunkProps* pFoundChunkProps = NULL;
	for( BcU32 i = 0; i < Header_.NoofChunks_; ++i )
	{
		CsFileChunk* pChunk = &pChunks_[ i ];
		BcU8* pOffset = ( (BcU8*)pData - (BcU8*)pData_ ) + (BcU8*)TotalHeaderSize;
		
		if( (BcU8*)pChunk->Offset_ == pOffset )
		{
			ChunkIdx = i;
			pFoundChunk = pChunk;
			pFoundChunkProps = &pChunkProps_[ i ];
			break;
		}
	}
	
	// We should have found it.
	BcAssert( pFoundChunk != NULL );
	
	// Set props as loaded and call delegate.
	if( pFoundChunk != NULL )
	{
		// Do a hash check of data.
		BcU32 Hash = (BcU32)BcHash( (BcU8*)pData, Size );



		BcAssertMsg( Hash == pFoundChunk->Hash_, "CsFileReader: Chunk data corrupt!\n" );
		
		pFoundChunkProps->Status_ = CsFileChunkProps::STATUS_LOADED;
		ChunkDelegate_( this, ChunkIdx, pFoundChunk, pData );
	}	
}



