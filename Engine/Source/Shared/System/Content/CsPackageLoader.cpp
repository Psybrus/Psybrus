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
#include "System/Content/CsSerialiserPackageObjectCodec.h"

#include "System/SysKernel.h"

#include "Serialisation/SeJsonReader.h"

#include "Base/BcCompression.h"

#include "Base/BcMath.h"

#include <boost/filesystem.hpp>

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPackageLoader::CsPackageLoader( CsPackage* pPackage, const BcPath& Path ):
	pPackage_( pPackage ),
	HasError_( BcFalse ),
	DataPosition_( 0 ),
	pPackageData_( NULL ),
	pStringTable_( NULL ),
	pResourceHeaders_( NULL ),
	pChunkHeaders_( NULL ),
	pChunkData_( NULL ),
	IsStringTableReady_( BcFalse ),
	IsDataLoaded_( BcFalse ),
	IsDataReady_( BcFalse ),
	PendingCallbackCount_( 0 )
{
	if( File_.open( (*Path).c_str(), fsFM_READ ) )
	{
#if PSY_IMPORT_PIPELINE
		// Load in package header synchronously to catch errors.
		BcU32 Bytes = sizeof( Header_ );
		++PendingCallbackCount_;
		File_.read( DataPosition_, &Header_, Bytes );
		DataPosition_ += Bytes;

		// Call on header.
		onHeaderLoaded( &Header_, Bytes );
#else
		// Load in package header asynchronously.
		BcU32 Bytes = sizeof( Header_ );
		++PendingCallbackCount_;
		File_.readAsync( DataPosition_, &Header_, Bytes, FsFileOpDelegate::bind< CsPackageLoader, &CsPackageLoader::onHeaderLoaded >( this ) );
		DataPosition_ += Bytes;
#endif
	}
	else
	{
		HasError_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPackageLoader::~CsPackageLoader()
{
	BcAssertMsg( hasPendingCallback() == BcFalse, "CsPackageLoader: Callbacks are pending." );

	File_.close();
	
	BcMemFree( pPackageData_ );
	pPackageData_ = NULL;

	// Release packages we reference.
	for( BcU32 Idx = 0; Idx < PackageDependencies_.size(); ++Idx )
	{
		PackageDependencies_[ Idx ]->release();
	}
}

//////////////////////////////////////////////////////////////////////////
// hasPendingCallback
BcBool CsPackageLoader::hasError() const
{
	return HasError_;
}

//////////////////////////////////////////////////////////////////////////
// hasPendingCallback
BcBool CsPackageLoader::hasPendingCallback() const
{
	return PendingCallbackCount_ != 0;
}

//////////////////////////////////////////////////////////////////////////
// isDataLoaded
BcBool CsPackageLoader::isDataLoaded() const
{
	return IsDataLoaded_;
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
	
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getCrossRefResource
CsResource* CsPackageLoader::getCrossRefResource( BcU32 Index )
{
	BcAssertMsg( Index < Header_.TotalPackageCrossRefs_, "CsPackageLoader: Invalid package cross ref index." );
	const CsPackageCrossRefData& PackageCrossRef( pPackageCrossRefs_[ Index ] );
	
	// If it's an ID, we just need to check the resource name vs resource index.
	if( PackageCrossRef.IsID_ )
	{
		return pPackage_->getResource( PackageCrossRef.ResourceName_ );
	}
	else
	{
		ReObjectRef< CsResource > Resource;
		CsPackage* pPackage = nullptr;
		BcName PackageName = getString( PackageCrossRef.PackageName_ );
		BcName ResourceName = getString( PackageCrossRef.ResourceName_ );
		BcName TypeName = getString( PackageCrossRef.TypeName_ );

		if( PackageCrossRef.IsWeak_ )
		{
			// Try find package, and check it's ready
			pPackage = CsCore::pImpl()->findPackage( PackageName );

			// If it's not ready or not loaded, return a nullptr resource. Up to the user to handle.
			if( pPackage == nullptr ||
				pPackage->isReady() == BcFalse )
			{
				return nullptr;
			}
		}
		else
		{
			// Request package, and check it's ready
			pPackage = CsCore::pImpl()->requestPackage( PackageName );
			BcAssertMsg( pPackage->isLoaded(), "CsPackageLoader: Package \"%s\" is not loaded, \"%s\" needs it loaded.", 
				(*PackageName).c_str(), (*pPackage_->getName()).c_str() );
		}

		// Find the resource.
		CsCore::pImpl()->internalFindResource( PackageName, ResourceName, ReManager::GetClass( *TypeName ), Resource );

		// If there is no valid resource at this point, then we must fail.
		BcAssertMsg( Resource.isValid(), "CsPackageLoader: Cross ref isn't valid!" );
			
		// Return resource.
		return Resource;
	}
}

//////////////////////////////////////////////////////////////////////////
// getCrossRefPackage
CsPackage* CsPackageLoader::getCrossRefPackage( BcU32 Index )
{
	BcAssertMsg( Index < Header_.TotalPackageCrossRefs_, "CsPackageLoader: Invalid package cross ref index." );
	const CsPackageCrossRefData& PackageCrossRef( pPackageCrossRefs_[ Index ] );
	BcName PackageName = getString( PackageCrossRef.PackageName_ );

	// Request package, return it.
	return CsCore::pImpl()->requestPackage( PackageName );
}

//////////////////////////////////////////////////////////////////////////
// getChunkSize
BcU32 CsPackageLoader::getChunkSize( BcU32 ResourceIdx, BcU32 ResourceChunkIdx )
{
	BcAssert( IsDataReady_ );

	CsPackageResourceHeader& ResourceHeader = pResourceHeaders_[ ResourceIdx ];
	BcU32 ChunkIdx = ResourceHeader.FirstChunk_ + ResourceChunkIdx;
	CsPackageChunkHeader& ChunkHeader = pChunkHeaders_[ ChunkIdx ];

	return ChunkHeader.UnpackedBytes_;
}

//////////////////////////////////////////////////////////////////////////
// getNoofChunks
BcU32 CsPackageLoader::getNoofChunks( BcU32 ResourceIdx )
{
	BcAssert( IsDataReady_ );

	CsPackageResourceHeader& ResourceHeader = pResourceHeaders_[ ResourceIdx ];

	return ( (BcU32)ResourceHeader.LastChunk_ - (BcU32)ResourceHeader.FirstChunk_ ) + 1;
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

	// Check the header is valid.
	if( Header_.Magic_ != CsPackageHeader::MAGIC )
	{
		BcPrintf( "CsPackageLoader: Invalid magic number. Not a valid package.\n" );
		HasError_ = BcTrue;
		--PendingCallbackCount_;
		return;
	}

	// Check version number.
	if( Header_.Version_ != CsPackageHeader::VERSION )
	{
		BcPrintf( "CsPackageLoader: Out of date package. Requires reimport.\n" );
		HasError_ = BcTrue;
		--PendingCallbackCount_;
		return;
	}

#if PSY_IMPORT_PIPELINE
	// Reimport if source file stats or dependencies have changed.
	const BcPath ImportPackage( CsCore::pImpl()->getPackageImportPath( pPackage_->getName() ) );

	// Read in dependencies.
	FsStats Stats;
	std::string OutputDependencies = *CsCore::pImpl()->getPackageIntermediatePath( pPackage_->getName() ) + "/deps.json";
	BcBool AnythingChanged = BcFalse;
	if( FsCore::pImpl()->fileStats( (*ImportPackage).c_str(), Stats ) )
	{
		AnythingChanged = ( Header_.SourceFileStatsHash_ != BcHash( reinterpret_cast< BcU8* >( &Stats ), sizeof( Stats ) ) );
	}

	if( boost::filesystem::exists( OutputDependencies ) )
	{
		CsPackageDependencies Dependencies;

		CsSerialiserPackageObjectCodec ObjectCodec( nullptr, (BcU32)bcRFF_ALL, (BcU32)bcRFF_TRANSIENT, 0 );
		SeJsonReader Reader( &ObjectCodec );
		Reader.load( OutputDependencies.c_str() );
		Reader << Dependencies;

		// Check other dependencies.
		if( !AnythingChanged )
		{
			for( const auto& Dependency : Dependencies.Dependencies_ )
			{
				if( Dependency.hasChanged() )
				{
					AnythingChanged = BcTrue;
					break;
				}
			}
		}
	}
	else
	{
		// No deps file, assume worst.
		AnythingChanged = BcTrue;
	}

	// Reimport.
	if( AnythingChanged )
	{
		BcPrintf( "CsPackageLoader: Source file stats have changed.\n" );
		HasError_ = BcTrue;
		--PendingCallbackCount_;
		return;
	}
#endif

	// Allocate all the memory we need up front.
	pPackageData_ = BcMemAlign( Header_.TotalAllocSize_, Header_.MaxAlignment_ );

	// Use this to advance as we need.
	BcU8* pCurrPackageData = reinterpret_cast< BcU8* >( pPackageData_ );
	
	// Loaded header, now markup the string table, chunks & props.
	pStringTable_ = reinterpret_cast< BcChar* >( pCurrPackageData );
	pCurrPackageData += BcCalcAlignment( Header_.StringTableBytes_, Header_.MinAlignment_ );

	pPackageCrossRefs_ = reinterpret_cast< CsPackageCrossRefData* >( pCurrPackageData );
	pCurrPackageData += BcCalcAlignment( Header_.TotalPackageCrossRefs_ * sizeof( CsPackageCrossRefData ), Header_.MinAlignment_ );

	pPackageDependencies_ = reinterpret_cast< CsPackageDependencyData* >( pCurrPackageData );
	pCurrPackageData += BcCalcAlignment( Header_.TotalPackageDependencies_ * sizeof( CsPackageDependencyData ), Header_.MinAlignment_ );

	pResourceHeaders_ = reinterpret_cast< CsPackageResourceHeader* >( pCurrPackageData );
	pCurrPackageData += BcCalcAlignment( Header_.TotalResources_ * sizeof( CsPackageResourceHeader ), Header_.MinAlignment_ );

	pChunkHeaders_ = reinterpret_cast< CsPackageChunkHeader* >( pCurrPackageData );
	pCurrPackageData += BcCalcAlignment( Header_.TotalChunks_ * sizeof( CsPackageChunkHeader ), Header_.MinAlignment_ );

	pChunkData_ = reinterpret_cast< CsPackageChunkData* >( pCurrPackageData );
	pCurrPackageData += BcCalcAlignment( Header_.TotalChunks_ * sizeof( CsPackageChunkData ), Header_.MinAlignment_ );

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
	
	// Load cross refs in.
	++PendingCallbackCount_;
	Bytes = Header_.TotalPackageCrossRefs_ * sizeof( CsPackageCrossRefData );
	File_.readAsync( DataPosition_, pPackageCrossRefs_, Bytes, FsFileOpDelegate::bind< CsPackageLoader, &CsPackageLoader::onPackageCrossRefsLoaded >( this ) );
	DataPosition_ += Bytes;

	// Load dependencies in.
	++PendingCallbackCount_;
	Bytes = Header_.TotalPackageDependencies_ * sizeof( CsPackageDependencyData );
	File_.readAsync( DataPosition_, pPackageDependencies_, Bytes, FsFileOpDelegate::bind< CsPackageLoader, &CsPackageLoader::onPackageDependenciesLoaded >( this ) );
	DataPosition_ += Bytes;

	// Load resources in.
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
// onPackageCrossRefsLoaded
void CsPackageLoader::onPackageCrossRefsLoaded( void* pData, BcSize Size )
{
	// Check we have the right data.
	BcAssert( pData == pPackageCrossRefs_ );
	BcAssert( Size == Header_.TotalPackageCrossRefs_ * sizeof( CsPackageCrossRefData ) );

	// This callback is complete.
	--PendingCallbackCount_;
}

//////////////////////////////////////////////////////////////////////////
// onPackageDependenciesLoaded
void CsPackageLoader::onPackageDependenciesLoaded( void* pData, BcSize Size )
{
	// Check we have the right data.
	BcAssert( pData == pPackageDependencies_ );
	BcAssert( Size == Header_.TotalPackageDependencies_ * sizeof( CsPackageDependencyData ) );

	// Request packages.
	PackageDependencies_.reserve( Header_.TotalPackageDependencies_ );
	for( BcU32 Idx = 0; Idx < Header_.TotalPackageDependencies_; ++Idx )
	{
		CsPackageDependencyData& PackageDependency( pPackageDependencies_[ Idx ] );

		if( PackageDependency.IsWeak_ == BcFalse )
		{
			CsPackage* pPackage = CsCore::pImpl()->requestPackage( getString( PackageDependency.PackageName_ ) );
			PackageDependencies_.push_back( pPackage );

			// Acquire package so it's not freed later.
			pPackage->acquire();
		}
	}

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

	// Data is loaded.
	IsDataLoaded_ = BcTrue;

	if( arePackageDependenciesReady() )
	{
		// Mark up all the resources.
		markupResources();

		// Data is ready.
		IsDataReady_ = BcTrue;

		// Now initialise resources.
		initialiseResources();
	}
	else
	{
		// Queue up callback for reentry next frame.
		BcDelegate< void (*)( void*, BcSize ) > Delegate( BcDelegate< void (*)( void*, BcSize ) >::bind< CsPackageLoader, &CsPackageLoader::onChunkHeadersLoaded >( this ) );
		SysKernel::pImpl()->enqueueCallback( Delegate, pData, Size );
		++PendingCallbackCount_;
	}

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
// markupResources
void CsPackageLoader::markupResources()
{
	// Use this to advance as we need.
	BcU8* pCurrPackageData = reinterpret_cast< BcU8* >( pPackageData_ ) + Header_.ResourceDataStart_;

	// Allocate chunks.
	for( BcU32 ChunkIdx = 0; ChunkIdx <= Header_.TotalChunks_; ++ChunkIdx )
	{
		CsPackageChunkHeader& ChunkHeader( pChunkHeaders_[ ChunkIdx ] );
		CsPackageChunkData& ChunkData( pChunkData_[ ChunkIdx ] );

		if( ChunkHeader.Flags_ & csPCF_MANAGED )
		{
			ChunkData.Managed_ = BcTrue;
			ChunkData.pUnpackedData_ = pCurrPackageData;

			// Advance package data.
 			pCurrPackageData += BcCalcAlignment( ChunkHeader.UnpackedBytes_, ChunkHeader.RequiredAlignment_ );

#ifdef PSY_DEBUG
			// Clear memory.
			BcMemSet( ChunkData.pUnpackedData_, 0x11, ChunkHeader.UnpackedBytes_ );
#endif
		}
		else
		{
			ChunkData.Managed_ = BcFalse;
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
		ReObjectRef< CsResource > Handle;
		auto Class = ReManager::GetClass( *Type );
		BcAssertMsg( Class, "CsPackageLoader: Unable to find class %s for resource", (*Type).c_str(), (*Name).c_str() );

		if( CsCore::pImpl()->internalCreateResource( Name, Class, ResourceIdx, pPackage_, Handle ) )
		{
			// Initialise.
			Handle->initialise();

			// Add to package.
			pPackage_->addResource( Handle );		

			// Tell it the file is ready (TODO: DEPRECATE).
			// NOTE: Will need to do this once we break out the import pipeline.
			Handle->fileReady();
		}
		else
		{
			// We can't create the resource.
			BcAssertMsg( BcFalse, "CsPackageLoader: Unable to create resource \"%s\" of type \"%s\".", (*Name).c_str(), (*Type).c_str() );
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

	BcU32 ResourceChunkIdx = ChunkIdx - ResourceHeader.FirstChunk_;
	BcAssert( ChunkIdx >= ResourceHeader.FirstChunk_ && ChunkIdx <= ResourceHeader.LastChunk_ );
		
	// Update the status.
	switch( ChunkData.Status_ )
	{
	case csPCS_NOT_LOADED:
		{
			// Ensure we've got a pointer for unpacked data.
			BcAssert( ChunkData.pUnpackedData_ != NULL );

			// Set status to loading.
			ChunkData.Status_ = csPCS_LOADING;

			// If we've got compressed data, read into it.
			if( ChunkHeader.Flags_ & csPCF_COMPRESSED )
			{
				// Chunk isn't loaded, need to read in data.
				BcU32 DataPosition = DataPosition_ + ChunkHeader.Offset_;
				BcU32 Bytes = ChunkHeader.PackedBytes_;
			
				//
				ChunkData.pPackedData_ = new BcU8[ Bytes ];

				// Do async read.
				++PendingCallbackCount_;
 				File_.readAsync( DataPosition, ChunkData.pPackedData_, Bytes, FsFileOpDelegate::bind< CsPackageLoader, &CsPackageLoader::onDataLoaded >( this ) );
			}
			else
			{
				// Chunk isn't loaded, need to read in data.
				BcU32 DataPosition = DataPosition_ + ChunkHeader.Offset_;
				BcU32 Bytes = ChunkHeader.UnpackedBytes_;
				
				// Do async read.
				++PendingCallbackCount_;
 				File_.readAsync( DataPosition, ChunkData.pUnpackedData_, Bytes, FsFileOpDelegate::bind< CsPackageLoader, &CsPackageLoader::onDataLoaded >( this ) );
			}
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
				BcUnusedVar( Hash );

				// Set status to ready.
				ChunkData.Status_ = csPCS_READY;
			}
			else
			{
				// Set status to unpacking.
				ChunkData.Status_ = csPCS_UNPACKING;

				// Check the data is valid.
				BcU32 Hash = BcHash( ChunkData.pPackedData_, ChunkHeader.PackedBytes_ );
				BcAssertMsg( Hash == ChunkHeader.PackedHash_, "Corrupted data." );
				BcUnusedVar( Hash );

				decompressChunk( ResourceIdx, ChunkIdx );
				return;
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
	CsResource* pResource = pPackage_->getResource( ResourceIdx );
	if( ChunkData.Status_ == csPCS_READY && pResource != NULL )
	{
		// Queue up callback.
		BcDelegate< void (*)( BcU32, BcU32, void* ) > Delegate( BcDelegate< void (*)( BcU32, BcU32, void* ) >::bind< CsResource, &CsResource::onFileChunkReady >( pResource ) );
		SysKernel::pImpl()->enqueueCallback( Delegate, ResourceChunkIdx, ChunkHeader.ID_, ChunkData.pUnpackedData_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// decompressChunk
void CsPackageLoader::decompressChunk( BcU32 ResourceIdx, BcU32 ChunkIdx )
{
	CsPackageResourceHeader& ResourceHeader = pResourceHeaders_[ ResourceIdx ];
	CsPackageChunkHeader& ChunkHeader = pChunkHeaders_[ ChunkIdx ];
	CsPackageChunkData& ChunkData = pChunkData_[ ChunkIdx ];

	BcU32 ResourceChunkIdx = ChunkIdx - ResourceHeader.FirstChunk_;
	BcAssert( ChunkIdx >= ResourceHeader.FirstChunk_ && ChunkIdx <= ResourceHeader.LastChunk_ );
		
	// Uncompress.
	if( BcDecompressData( ChunkData.pPackedData_, ChunkHeader.PackedBytes_, ChunkData.pUnpackedData_, ChunkHeader.UnpackedBytes_ ) )
	{
		// Done, free packed data.
		delete [] ChunkData.pPackedData_;
		ChunkData.pPackedData_ = NULL;
					
		// Set status to ready.
		ChunkData.Status_ = csPCS_READY;
	}
	else
	{
		BcBreakpoint;
	}

	// If state has changed to ready, do callback.
	CsResource* pResource = pPackage_->getResource( ResourceIdx );
	if( ChunkData.Status_ == csPCS_READY && pResource != NULL )
	{
		// Queue up callback.
		BcDelegate< void (*)( BcU32, BcU32, void* ) > Delegate( BcDelegate< void (*)( BcU32, BcU32, void* ) >::bind< CsResource, &CsResource::onFileChunkReady >( pResource ) );
		SysKernel::pImpl()->enqueueCallback( Delegate, ResourceChunkIdx, ChunkHeader.ID_, ChunkData.pUnpackedData_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// arePackageDependenciesReady
BcBool CsPackageLoader::arePackageDependenciesReady()
{
	for( BcU32 Idx = 0; Idx < PackageDependencies_.size(); ++Idx )
	{
		if( PackageDependencies_[ Idx ]->isReady() == BcFalse )
		{
			return BcFalse;
		}
	}
	
	return BcTrue;
}
