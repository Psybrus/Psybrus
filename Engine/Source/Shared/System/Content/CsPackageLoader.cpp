/**************************************************************************
*
* File:		CsPackageLoader.cpp
* Author:	Neil Richardson
* Ver/Date:	8/04/12
* Description:
*
*
*
*
*
**************************************************************************/

#include "System/Content/CsPackageLoader.h"

#include "System/Content/CsCore.h"

#include "System/SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPackageLoader::CsPackageLoader( CsPackage* pPackage, const BcPath& Path ):
	pPackage_( pPackage ),
	DataPosition_( 0 ),
	pStringTable_( NULL ),
	pResourceHeaders_( NULL ),
	pChunkHeaders_( NULL ),
	pChunkData_( NULL ),
	IsStringTableReady_( BcFalse ),
	IsDataReady_( BcFalse )
{
	if( File_.open( (*Path).c_str(), fsFM_READ ) )
	{
		// Load in package header.
		BcU32 Bytes = sizeof( Header_ );
		++PendingCallbackCount_;
		File_.readAsync( DataPosition_, &Header_, Bytes, FsFileOpDelegate::bind< CsPackageLoader, &CsPackageLoader::onHeaderLoaded >( this ) );
		DataPosition_ += Bytes;
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPackageLoader::~CsPackageLoader()
{
	BcAssertMsg( hasPendingCallback() == BcFalse, "CsPackageLoader: Callbacks are pending." );

	File_.close();
	freeResourceData();
}

//////////////////////////////////////////////////////////////////////////
// hasPendingCallback
BcBool CsPackageLoader::hasPendingCallback() const
{
	return PendingCallbackCount_ != 0;
}

//////////////////////////////////////////////////////////////////////////
// isDataReady
BcBool CsPackageLoader::isDataReady() const
{
	return IsDataReady_;
}

//////////////////////////////////////////////////////////////////////////
// getSourceFile
const BcChar* CsPackageLoader::getSourceFile() const
{
	BcAssert( IsDataReady_ );

	return getString( Header_.SourceFile_ );
}

//////////////////////////////////////////////////////////////////////////
// getString
const BcChar* CsPackageLoader::getString( BcU32 Offset ) const
{
	BcAssert( IsStringTableReady_ );

	if( Offset < Header_.StringTableBytes_ )
	{
		return &pStringTable_[ Offset ];
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getChunkSize
BcU32 CsPackageLoader::getChunkSize( BcU32 ResourceIdx, BcU32 ResourceChunkIdx )
{
	BcAssert( IsDataReady_ );

	CsPackageResourceHeader& ResourceHeader = pResourceHeaders_[ ResourceIdx ];
	BcU32 ChunkIdx = ResourceHeader.FirstChunk_ + ResourceChunkIdx;
	CsPackageChunkHeader& ChunkHeader = pChunkHeaders_[ ChunkIdx ];
	CsPackageChunkData& ChunkData = pChunkData_[ ChunkIdx ];

	return ChunkHeader.UnpackedBytes_;
}

//////////////////////////////////////////////////////////////////////////
// getNoofChunks
BcU32 CsPackageLoader::getNoofChunks( BcU32 ResourceIdx )
{
	BcAssert( IsDataReady_ );

	CsPackageResourceHeader& ResourceHeader = pResourceHeaders_[ ResourceIdx ];

	return ( ResourceHeader.LastChunk_ - ResourceHeader.FirstChunk_ ) + 1;
}

//////////////////////////////////////////////////////////////////////////
// requestChunk
BcBool CsPackageLoader::requestChunk( BcU32 ResourceIdx, BcU32 ResourceChunkIdx, void* pDataLocation )
{
	BcAssert( IsDataReady_ );
	
	CsPackageResourceHeader& ResourceHeader = pResourceHeaders_[ ResourceIdx ];
	BcU32 ChunkIdx = ResourceHeader.FirstChunk_ + ResourceChunkIdx;
	CsPackageChunkHeader& ChunkHeader = pChunkHeaders_[ ChunkIdx ];
	CsPackageChunkData& ChunkData = pChunkData_[ ChunkIdx ];
	
	// Handle unmanaged data.
	if( ( ChunkHeader.Flags_ & csPCF_MANAGED ) == 0 )
	{
		BcAssertMsg( pDataLocation != NULL, "CsPackageLoader: Unmanaged chunks require a data location." );
		ChunkData.pUnpackedData_ = reinterpret_cast< BcU8* >( pDataLocation );
	}	
	
	// If the chunk isn't loaded, then process (this will kick off the load).
	if( ChunkData.Status_ == csPCS_NOT_LOADED || ChunkData.Status_ == csPCS_READY )
	{
		processResourceChunk( ResourceIdx, ChunkIdx );
		return BcTrue;
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// onHeaderLoaded
void CsPackageLoader::onHeaderLoaded( void* pData, BcSize Size )
{
	// Check we have the right data.
	BcAssert( pData == &Header_ );
	BcAssert( Size == sizeof( Header_ ) );
	
	// Loaded header, now allocate the string table, chunks & props.
	pStringTable_ = new BcChar[ Header_.StringTableBytes_ ];
	pResourceHeaders_ = new CsPackageResourceHeader[ Header_.TotalResources_ ];
	pChunkHeaders_ = new CsPackageChunkHeader[ Header_.TotalChunks_ ];
	pChunkData_ = new CsPackageChunkData[ Header_.TotalChunks_ ];

	// Clear string table.
	BcMemZero( pStringTable_, Header_.StringTableBytes_ );
	
	// Clear chunk data.
	for( BcU32 Idx = 0; Idx < Header_.TotalChunks_; ++Idx )
	{
		pChunkData_[ Idx ].Status_ = csPCS_NOT_LOADED;	
		pChunkData_[ Idx ].Managed_ = BcFalse;
		pChunkData_[ Idx ].pPackedData_ = NULL;
		pChunkData_[ Idx ].pUnpackedData_ = NULL;
	}

	// Setup file position data.
	BcU32 Bytes = 0;
	
	// Load the string table in.
	++PendingCallbackCount_;
	Bytes = Header_.StringTableBytes_;
	File_.readAsync( DataPosition_, pStringTable_, Bytes, FsFileOpDelegate::bind< CsPackageLoader, &CsPackageLoader::onStringTableLoaded >( this ) );
	DataPosition_ += Bytes;
	
	// Load Resources in.
	++PendingCallbackCount_;
	Bytes = Header_.TotalResources_ * sizeof( CsPackageResourceHeader );
	File_.readAsync( DataPosition_, pResourceHeaders_, Bytes, FsFileOpDelegate::bind< CsPackageLoader, &CsPackageLoader::onResourceHeadersLoaded >( this ) );
	DataPosition_ += Bytes;

	// Load chunks in.
	++PendingCallbackCount_;
	Bytes = Header_.TotalChunks_ * sizeof( CsPackageChunkHeader );
	File_.readAsync( DataPosition_, pChunkHeaders_, Bytes, FsFileOpDelegate::bind< CsPackageLoader, &CsPackageLoader::onChunkHeadersLoaded >( this ) );
	DataPosition_ += Bytes;

	// This callback is complete.
	--PendingCallbackCount_;
}

//////////////////////////////////////////////////////////////////////////
// onStringTableLoaded
void CsPackageLoader::onStringTableLoaded( void* pData, BcSize Size )
{
	// Check we have the right data.
	BcAssert( pData == pStringTable_ );
	BcAssert( Size == Header_.StringTableBytes_ );

	// String table is ready.
	IsStringTableReady_ = BcTrue;

	// This callback is complete.
	--PendingCallbackCount_;
}

//////////////////////////////////////////////////////////////////////////
// onResourceHeadersLoaded
void CsPackageLoader::onResourceHeadersLoaded( void* pData, BcSize Size )
{
	// Check we have the right data.
	BcAssert( pData == pResourceHeaders_ );
	BcAssert( Size == sizeof( CsPackageResourceHeader ) * Header_.TotalResources_ );

	// This callback is complete.
	--PendingCallbackCount_;
}

//////////////////////////////////////////////////////////////////////////
// onChunkHeadersLoaded
void CsPackageLoader::onChunkHeadersLoaded( void* pData, BcSize Size )
{
	// Check we have the right data.
	BcAssert( pData == pChunkHeaders_ );
	BcAssert( Size == sizeof( CsPackageChunkHeader ) * Header_.TotalChunks_ );

	// We've got all the data we need to alloc resource data.
	allocResourceData();
	
	// Data is ready.
	IsDataReady_ = BcTrue;

	// Now initialise resources.
	initialiseResources();

	// This callback is complete.
	--PendingCallbackCount_;
}

//////////////////////////////////////////////////////////////////////////
// onDataLoaded
void CsPackageLoader::onDataLoaded( void* pData, BcSize Size )
{
	BcU32 ResourceIdx = 0;
	BcU32 ChunkIdx = 0;

	// If we can find the chunk.
	if( findResourceChunk( pData, ResourceIdx, ChunkIdx ) )
	{
		processResourceChunk( ResourceIdx, ChunkIdx );
	}

	// This callback is complete.
	--PendingCallbackCount_;
}

//////////////////////////////////////////////////////////////////////////
// allocResourceData
void CsPackageLoader::allocResourceData()
{
	// For all resources, allocate the memory they all require.
	for( BcU32 ResourceIdx = 0; ResourceIdx < Header_.TotalResources_; ++ResourceIdx )
	{
		CsPackageResourceHeader& ResourceHeader = pResourceHeaders_[ ResourceIdx ];
		
		// Handle contiguous resources first.
		if( ResourceHeader.Flags_ & csPEF_CONTIGUOUS_CHUNKS )
		{
			BcU32 FirstManagedChunk = BcErrorCode;
			BcU32 TotalChunkSize = 0;
			
			// Iterate over chunks.
			for( BcU32 ChunkIdx = ResourceHeader.FirstChunk_; ChunkIdx <= ResourceHeader.LastChunk_; ++ChunkIdx )
			{
				CsPackageChunkHeader& ChunkHeader( pChunkHeaders_[ ChunkIdx ] );

				// If chunk is managed, take it's size into account.
				if( ChunkHeader.Flags_ & csPCF_MANAGED )
				{
					if( FirstManagedChunk == BcErrorCode )
					{
						FirstManagedChunk = ChunkIdx;
					}

					TotalChunkSize += ChunkHeader.UnpackedBytes_;
				}
				else
				{
					// Unmanaged chunk, allocate and write out chunks we've passed over.
					if( FirstManagedChunk != BcErrorCode && TotalChunkSize > 0 )
					{
						BcU8* pData = new BcU8[ TotalChunkSize ];
						for( BcU32 ManagedChunkIdx = FirstManagedChunk; ManagedChunkIdx < ChunkIdx; ++ManagedChunkIdx )
						{
							CsPackageChunkHeader& ManagedChunkHeader( pChunkHeaders_[ ManagedChunkIdx ] );						
							CsPackageChunkData& ManagedChunkData( pChunkData_[ ManagedChunkIdx ] );
							
							ManagedChunkData.pUnpackedData_ = pData;
							ManagedChunkData.Managed_ = ManagedChunkIdx == FirstManagedChunk;

							// Advance data pointer.
							pData += ManagedChunkHeader.UnpackedBytes_;
						}

						// Reset chunk.
						FirstManagedChunk = BcErrorCode;
						TotalChunkSize = 0;
					}
				}
			}

			// Allocate chunk if we've still got a chunk pending.
			if( FirstManagedChunk != BcErrorCode && TotalChunkSize > 0 )
			{
				BcU8* pData = new BcU8[ TotalChunkSize ];
				for( BcU32 ManagedChunkIdx = FirstManagedChunk; ManagedChunkIdx <= ResourceHeader.LastChunk_; ++ManagedChunkIdx )
				{
					CsPackageChunkHeader& ManagedChunkHeader( pChunkHeaders_[ ManagedChunkIdx ] );						
					CsPackageChunkData& ManagedChunkData( pChunkData_[ ManagedChunkIdx ] );
							
					ManagedChunkData.pUnpackedData_ = pData;
					ManagedChunkData.Managed_ = ManagedChunkIdx == FirstManagedChunk;

					// Advance data pointer.
					pData += ManagedChunkHeader.UnpackedBytes_;
				}

				// Reset chunk.
				FirstManagedChunk = BcErrorCode;
				TotalChunkSize = 0;
			}
		}
		else
		{
			// Allocate chunks.
			for( BcU32 ChunkIdx = ResourceHeader.FirstChunk_; ChunkIdx <= ResourceHeader.LastChunk_; ++ChunkIdx )
			{
				CsPackageChunkHeader& ChunkHeader( pChunkHeaders_[ ChunkIdx ] );
				CsPackageChunkData& ChunkData( pChunkData_[ ChunkIdx ] );

				ChunkData.Managed_ = BcTrue;
				ChunkData.pUnpackedData_ = new BcU8[ ChunkHeader.UnpackedBytes_ ];
				
				// Clear memory.
				BcMemSet( ChunkData.pUnpackedData_, 0x11, ChunkHeader.UnpackedBytes_ );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// initialiseResources
void CsPackageLoader::initialiseResources()
{
	// Create all resources, and add them to the parent package.
	for( BcU32 ResourceIdx = 0; ResourceIdx < Header_.TotalResources_; ++ResourceIdx )
	{
		CsPackageResourceHeader& ResourceHeader = pResourceHeaders_[ ResourceIdx ];

		// Grab name and type.
		BcName Name( getString( ResourceHeader.Name_ ) );
		BcName Type( getString( ResourceHeader.Type_ ) );

		// Allocate resource, and signal ready.
		CsResourceRef<> Handle;
		if( CsCore::pImpl()->internalCreateResource( Name, Type, ResourceIdx, pPackage_, Handle ) )
		{
			// Initialise.
			Handle->initialise();

			// Add to package.
			pPackage_->addResource( Handle );		

			// Tell it the file is ready (TODO: DEPRECATE).
			Handle->fileReady();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// freeResourceData
void CsPackageLoader::freeResourceData()
{
	// For all resources, allocate the memory they all require.
	for( BcU32 ResourceIdx = 0; ResourceIdx < Header_.TotalResources_; ++ResourceIdx )
	{
		CsPackageResourceHeader& ResourceHeader = pResourceHeaders_[ ResourceIdx ];
		
		// Allocate chunks.
		for( BcU32 ChunkIdx = ResourceHeader.FirstChunk_; ChunkIdx <= ResourceHeader.LastChunk_; ++ChunkIdx )
		{
			CsPackageChunkHeader& ChunkHeader( pChunkHeaders_[ ChunkIdx ] );
			CsPackageChunkData& ChunkData( pChunkData_[ ChunkIdx ] );
			
			// If the chunk data is managed, delete it.
			if( ChunkData.Managed_ )
			{
				delete [] ChunkData.pUnpackedData_;
			}
			
			// Clear.
			ChunkData.pUnpackedData_ = NULL;
			ChunkData.Managed_ = BcFalse;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// findResourceChunk
BcBool CsPackageLoader::findResourceChunk( void* pData, BcU32& ResourceIdx, BcU32& ChunkIdx )
{
	for( ResourceIdx = 0; ResourceIdx < Header_.TotalResources_; ++ResourceIdx )
	{
		CsPackageResourceHeader& ResourceHeader( pResourceHeaders_[ ResourceIdx ] );
		
		for( ChunkIdx = ResourceHeader.FirstChunk_; ChunkIdx <= ResourceHeader.LastChunk_; ++ChunkIdx )
		{
			CsPackageChunkData& ChunkData( pChunkData_[ ChunkIdx ] );

			// If the pointer matches the packed or unpacked data
			if( ChunkData.pPackedData_ == pData || ChunkData.pUnpackedData_ == pData )
			{
				return BcTrue;
			}
		}
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// processResourceChunk
void CsPackageLoader::processResourceChunk( BcU32 ResourceIdx, BcU32 ChunkIdx )
{
	CsPackageResourceHeader& ResourceHeader = pResourceHeaders_[ ResourceIdx ];
	CsPackageChunkHeader& ChunkHeader = pChunkHeaders_[ ChunkIdx ];
	CsPackageChunkData& ChunkData = pChunkData_[ ChunkIdx ];

	CsResource* pResource = pPackage_->getResource( ResourceIdx );
	BcU32 ResourceChunkIdx = ChunkIdx - ResourceHeader.FirstChunk_;
	BcAssert( pResource != NULL );
	BcAssert( ChunkIdx >= ResourceHeader.FirstChunk_ && ChunkIdx <= ResourceHeader.LastChunk_ );
		
	// Update the status.
	switch( ChunkData.Status_ )
	{
	case csPCS_NOT_LOADED:
		{
			// TODO: Handle compressed data later.
			BcAssert( ( ChunkHeader.Flags_ & csPCF_COMPRESSED ) == 0 );

			// Ensure we've got a pointer for unpacked data.
			BcAssert( ChunkData.pUnpackedData_ != NULL );

			// Set status to loading.
			ChunkData.Status_ = csPCS_LOADING;

			// Chunk isn't loaded, need to read in data.
			BcU32 DataPosition = DataPosition_ + ChunkHeader.Offset_;
			BcU32 Bytes = ChunkHeader.UnpackedBytes_;
			
			// Do async read.
			++PendingCallbackCount_;
 			File_.readAsync( DataPosition, ChunkData.pUnpackedData_, Bytes, FsFileOpDelegate::bind< CsPackageLoader, &CsPackageLoader::onDataLoaded >( this ) );
		}
		break;

	case csPCS_LOADING:
		{
			// If this is a compressed chunk, we need to move to the unpacking stage first.
			if( ( ChunkHeader.Flags_ & csPCF_COMPRESSED ) == 0 )
			{
				// Check the data is valid.
				BcU32 Hash = BcHash( ChunkData.pUnpackedData_, ChunkHeader.UnpackedBytes_ );
				BcAssertMsg( Hash == ChunkHeader.UnpackedHash_, "Corrupted data." );

				// Set status to ready.
				ChunkData.Status_ = csPCS_READY;
			}
			else
			{
				// Set status to unpacking.
				ChunkData.Status_ = csPCS_UNPACKING;

				// TODO: Async decompress.
			}
		}
		break;
		
	case csPCS_UNPACKING:
		{
			// TODO: Unpacking is complete, free packed data.
			ChunkData.Status_ = csPCS_READY;
		}
		break;
		
	case csPCS_READY:
		{
			// Don't need to do any processing.
		}
		break;
	}


	// If state has changed to ready, do callback.
	if( ChunkData.Status_ == csPCS_READY )
	{
		// Queue up callback.
		BcDelegate< void (*)( BcU32, BcU32, void* ) > Delegate( BcDelegate< void (*)( BcU32, BcU32, void* ) >::bind< CsResource, &CsResource::onFileChunkReady >( pResource ) );
		SysKernel::pImpl()->enqueueCallback( Delegate, ResourceChunkIdx, ChunkHeader.ID_, ChunkData.pUnpackedData_ );
	}
}
