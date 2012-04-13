/**************************************************************************
*
* File:		CsPackageImporter.cpp
* Author:	Neil Richardson
* Ver/Date:	8/04/12
* Description:
*
*
*
*
*
**************************************************************************/

#include "System/Content/CsPackageImporter.h"

#include "System/Content/CsCore.h"

#include "Base/BcStream.h"
#include "Base/BcCompression.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPackageImporter::CsPackageImporter()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPackageImporter::~CsPackageImporter()
{
	// Delete all working data.
	for( BcU32 Idx = 0; Idx < ChunkDatas_.size(); ++Idx )
	{
		CsPackageChunkData& ChunkData = ChunkDatas_[ Idx ];

		delete ChunkData.pPackedData_;
		delete ChunkData.pUnpackedData_;
	}

	//
	StringList_.clear();
	ResourceHeaders_.clear();
	ChunkHeaders_.clear();
	ChunkDatas_.clear();
}

//////////////////////////////////////////////////////////////////////////
// import
BcBool CsPackageImporter::import( const BcName& Name )
{
	BcPath Path = CsCore::pImpl()->getPackageImportPath( Name );

	BcPrintf( "============================================================================\n" );
	BcPrintf( "CsPackageImporter: Importing %s...\n", (*Path).c_str() );

	// Store source package to header.
	Header_.SourceFile_ = addString( (*Path).c_str() );

	Json::Value Root;
	if( loadJsonFile( (*Path).c_str(), Root ) )
	{
		// Get resource list.
		Json::Value Resources( Root.get( "resources", Json::Value( Json::arrayValue ) ) );

		// Add resources to import list.
		for( BcU32 Idx = 0; Idx < Resources.size(); ++Idx )
		{
			JsonResources_.push_back( Resources[ Idx ] );
		}

		// Iterate over all resources and import (import calls can append to the list)
		while( JsonResources_.size() > 0 )
		{
			// Grab first resource in the list.
			Json::Value ResourceObject = *JsonResources_.begin();
			JsonResources_.pop_front();
			
			// Import resource.
			if( importResource( ResourceObject ) )
			{
				BcPrintf( " - - Succeeded.\n" );
			}
			else
			{
				BcPrintf( " - - FAILED.\n" );
			}
		}
		
		// Save and return.
		BcPath PackedPackage( CsCore::pImpl()->getPackagePackedPath( Name ) );
		return save( PackedPackage );
	}
	
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// save
BcBool CsPackageImporter::save( const BcPath& Path )
{
	// Open package output.
	// TODO: Write to temp file first, then move.
	if( File_.open( (*Path).c_str(), bcFM_WRITE ) )
	{
		// Generate string table.
		BcStream StringTableStream;

		for( BcU32 Idx = 0; Idx < StringList_.size(); ++Idx )
		{
			const std::string& StringEntry( StringList_[ Idx ] );
			StringTableStream.push( StringEntry.c_str(), StringEntry.size() + 1 );
		}	

		// Setup header.
		Header_.Magic_ = CsPackageHeader::MAGIC;
		Header_.StringTableBytes_ = StringTableStream.dataSize();
		Header_.TotalResources_ = ResourceHeaders_.size();
		Header_.TotalChunks_ = ChunkHeaders_.size();
		Header_.TotalAllocSize_ = 0;
		Header_.MinAlignment_ = 16;		// TODO: Platform specific.
		Header_.MaxAlignment_ = 4096;	// TODO: Platform specific.

		// Calculate package alloc size.
		Header_.TotalAllocSize_ += StringTableStream.dataSize();
		Header_.TotalAllocSize_ += BcCalcAlignment( ResourceHeaders_.size() + sizeof( CsPackageResourceHeader ), Header_.MinAlignment_ );
		Header_.TotalAllocSize_ += BcCalcAlignment( ChunkHeaders_.size() + sizeof( CsPackageChunkHeader ), Header_.MinAlignment_ );
		Header_.TotalAllocSize_ += BcCalcAlignment( ChunkHeaders_.size() + sizeof( CsPackageChunkData ), Header_.MinAlignment_ );
		
		// Align total size to 1 page for the start of resource data.
		Header_.TotalAllocSize_ = BcCalcAlignment( Header_.TotalAllocSize_, Header_.MaxAlignment_ );

		// Set start of resource data.
		Header_.ResourceDataStart_ = Header_.TotalAllocSize_;

		// Add resource sizes.
		for( BcU32 Idx = 0; Idx < ChunkHeaders_.size(); ++Idx )
		{
			const CsPackageChunkHeader& ChunkHeader = ChunkHeaders_[ Idx ];
			if( ChunkHeader.Flags_ & csPCF_MANAGED )
			{
				Header_.TotalAllocSize_ += BcCalcAlignment( ChunkHeader.UnpackedBytes_, ChunkHeader.RequiredAlignment_ );
			}
		}
		
		// Write header.
		File_.write( &Header_, sizeof( Header_ ) );
		
		// Write string table.
		File_.write( StringTableStream.pData(), StringTableStream.dataSize() );

		// Write resource headers.
		for( BcU32 Idx = 0; Idx < ResourceHeaders_.size(); ++Idx )
		{
			CsPackageResourceHeader& ResourceHeader = ResourceHeaders_[ Idx ];
			File_.write( &ResourceHeader, sizeof( CsPackageResourceHeader ) );
		}

		// Write chunk headers.
		BcU32 Offset = 0;
		for( BcU32 Idx = 0; Idx < ChunkHeaders_.size(); ++Idx )
		{
			CsPackageChunkHeader& ChunkHeader = ChunkHeaders_[ Idx ];

			// Setup chunk offset.
			ChunkHeader.Offset_ = Offset;
			Offset += ChunkHeader.PackedBytes_;
			
			//
			File_.write( &ChunkHeader, sizeof( CsPackageChunkHeader ) );
		}

		// Write out all chunk data.
		for( BcU32 Idx = 0; Idx < ChunkDatas_.size(); ++Idx )
		{
			const CsPackageChunkHeader& ChunkHeader = ChunkHeaders_[ Idx ];
			const CsPackageChunkData& ChunkData = ChunkDatas_[ Idx ];
			
			File_.write( ChunkData.pPackedData_, ChunkHeader.PackedBytes_ );
		}

		// Close file.
		File_.close();

		//
		return BcTrue;
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// loadJsonFile
BcBool CsPackageImporter::loadJsonFile( const BcChar* pFileName, Json::Value& Root )
{
	BcBool Success = BcFalse;
	BcFile File;
	if( File.open( pFileName ) )
	{
		char* pData = new char[ File.size() ];
		File.read( pData, File.size() );
		
		Json::Reader Reader;
		
		if( Reader.parse( pData, pData + File.size(), Root ) )
		{
			Success = BcTrue;
		}
		else
		{
			BcPrintf( "CsPackageImporter: Failed to parse Json:\n %s\n", Reader.getFormatedErrorMessages().c_str() );
		}
		
		delete [] pData;
	}
	
	return Success;
}

//////////////////////////////////////////////////////////////////////////
// importResource
BcBool CsPackageImporter::importResource( const Json::Value& Resource )
{
	BcAssertMsg( Resource.type() == Json::objectValue, "CsPackageImporter: Can't import a value that isn't an object." );
	Json::Value Name( Resource.get( "name", Json::Value( Json::nullValue ) ) );
	Json::Value Type( Resource.get( "type", Json::Value( Json::nullValue ) ) );

	// Catch name being missing.
	if( Name.type() != Json::stringValue )
	{
		BcPrintf( "- importResource: Name not specified for resource.\n" );
		return BcFalse;
	}

	// Catch type being missing.
	if( Name.type() != Json::stringValue )
	{
		BcPrintf( "- importResource: Type not specified for resource.\n" );
		return BcFalse;
	}

	BcPrintf( " - importResource: Processing \"%s\" of type \"%s\"\n", Name.asCString(),  Type.asCString() );

	// Get resource index.
	BcU32 ResourceIndex = ResourceHeaders_.size();

	// Get first chunk used by resource.
	BcU32 FirstChunk = ChunkHeaders_.size();

	// Allocate a resource (TODO: Use static member for import instead of instance.)
	CsResource* pResource = CsCore::pImpl()->allocResource( Name.asCString(), Type.asCString(), ResourceIndex, NULL );
	if( pResource == NULL )
	{
		BcPrintf( "CsPackageImporter: Can't allocate resource \"%s\" of type \"%s\".\n", Name.asCString(), Type.asCString() );
		return BcFalse;
	}
	
	// Call resource import.
	BcBool SuccessfulImport = pResource->import( *this, Resource );

	if( SuccessfulImport )
	{
		// Setup current resource header.
		CurrResourceHeader_.Name_ = addString( Name.asCString() );
		CurrResourceHeader_.Type_ = addString( Type.asCString() );
		CurrResourceHeader_.Flags_ = csPEF_CONTIGUOUS_CHUNKS;
		CurrResourceHeader_.FirstChunk_ = FirstChunk;
		CurrResourceHeader_.LastChunk_ = ChunkHeaders_.size() - 1; // Assumes 1 chunk for resource. Fair assumption.
		
		// Make sure chunk indices are valid.
		BcAssert( CurrResourceHeader_.FirstChunk_ <= CurrResourceHeader_.LastChunk_ );

		ResourceHeaders_.push_back( CurrResourceHeader_ );
	}
	
	return SuccessfulImport;
}

//////////////////////////////////////////////////////////////////////////
// addImport
void CsPackageImporter::addImport( const Json::Value& Resource )
{
	// Validate it's an object.
	BcAssertMsg( Resource.type() == Json::objectValue, "CsPackageImporter: Can't import a value that isn't an object." );

	// Validate name and type.
	Json::Value Name( Resource.get( "name", Json::Value( Json::nullValue ) ) );
	Json::Value Type( Resource.get( "type", Json::Value( Json::nullValue ) ) );
	BcAssertMsg( Name.type() == Json::stringValue, "CsPackageImporter: Name not specified for resource.\n" );
	BcAssertMsg( Type.type() == Json::stringValue, "CsPackageImporter: Type not specified for resource.\n" )

	// Put to front of list so it's imported next.
	JsonResources_.push_front( Resource );
}

//////////////////////////////////////////////////////////////////////////
// addString
BcU32 CsPackageImporter::addString( const BcChar* pString )
{
	BcU32 CurrentOffset = 0;

	for( BcU32 Idx = 0; Idx < StringList_.size(); ++Idx )
	{
		const std::string& StringEntry( StringList_[ Idx ] );

		if( StringEntry == pString )
		{
			return CurrentOffset;
		}

		// String length with null terminator.
		CurrentOffset += StringEntry.length() + 1;
	}

	// Add string to list.
	StringList_.push_back( pString );

	// Return current offset.
	return CurrentOffset;
}

//////////////////////////////////////////////////////////////////////////
// addChunk
BcU32 CsPackageImporter::addChunk( BcU32 ID, void* pData, BcU32 Size, BcU32 RequiredAlignment, BcU32 Flags )
{
	BcAssert( Size > 0 );
	BcAssert( BcPot( RequiredAlignment ) );
	BcAssert( RequiredAlignment <= 4096 );

	// Setup default packed data.
	BcU8* pPackedData = static_cast< BcU8* >( pData );
	BcU32 PackedSize = Size;

	// If we need to compress, do so.
	if( ( Flags & csPCF_COMPRESSED ) != 0 )
	{
		if( BcCompressData( static_cast< BcU8* >( pData ), Size, pPackedData, PackedSize ) )
		{
			// Do nothing.
		}
		else
		{
			// On failure, strip compressed flag.
			Flags &= ~csPCF_COMPRESSED;
		}
	}
	
	// Generate header.
	CsPackageChunkHeader ChunkHeader;
	ChunkHeader.ID_ = ID;
	ChunkHeader.Offset_ = 0;
	ChunkHeader.Flags_ = Flags;
	ChunkHeader.RequiredAlignment_ = RequiredAlignment;
	ChunkHeader.PackedBytes_ = PackedSize;
	ChunkHeader.UnpackedBytes_ = Size;
	ChunkHeader.PackedHash_ = BcHash( (BcU8*)pPackedData, PackedSize );
	ChunkHeader.UnpackedHash_ = BcHash( (BcU8*)pData, Size );

	// Generate data.
	CsPackageChunkData ChunkData;
	ChunkData.Status_ = csPCS_NOT_LOADED;
	ChunkData.Managed_ = BcFalse;
	
	// Store as packed data.
	ChunkData.pPackedData_ = new BcU8[ PackedSize ];
	ChunkData.pUnpackedData_ = NULL;
	BcMemCopy( ChunkData.pPackedData_, pPackedData, PackedSize );

	// Push into lists.
	ChunkHeaders_.push_back( ChunkHeader );
	ChunkDatas_.push_back( ChunkData );
	return ChunkHeaders_.size() - 1;
}

//////////////////////////////////////////////////////////////////////////
// addDependency
void CsPackageImporter::addDependency( const BcChar* pFileName )
{
	BcUnusedVar( pFileName );
}
