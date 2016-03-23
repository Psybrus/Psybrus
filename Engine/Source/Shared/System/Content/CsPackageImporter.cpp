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
#include "System/Content/CsResourceImporter.h"


#include "System/Content/CsCore.h"
#include "System/Content/CsSerialiserPackageObjectCodec.h"

#include "System/File/FsCore.h"

#include "Reflection/ReReflection.h"
#include "Serialisation/SeJsonWriter.h"
#include "Serialisation/SeJsonReader.h"

#include "Base/BcCompression.h"
#include "Base/BcFile.h"
#include "Base/BcStream.h"

#include "Base/BcMath.h"

#include "System/SysKernel.h"

#include <regex>
#include <iostream>
#include <sstream>

#undef ERROR

//////////////////////////////////////////////////////////////////////////
// Regex for resource references.
static std::regex GRegex_ResourceReference( "^\\$\\((.*?):(.*?)\\.(.*?)\\)" );		// Matches "$(Type:Package.Resource)"
static std::regex GRegex_WeakResourceReference( "^\\#\\((.*?):(.*?)\\.(.*?)\\)" );	// Matches "#(Type:Package.Resource)" // TODO: Merge into the ResourceReference regex.
static std::regex GRegex_ResourceIdReference( "^\\$\\((.*?)\\)" );					// Matches "$(ID)".
static std::regex GRegex_Filter( "^@\\((.*?)\\)" );								// Matches "(String)".

//////////////////////////////////////////////////////////////////////////
// Anonymous namespace
namespace 
{
	/**
	 * Process filters in Json. Will merge down, and remove filter blocks as appropriate.
	 */
	void ProcessFiltersOnJson( const CsPlatformParams& Params, Json::Value & Value )
	{
		// Recurse down.
		for( auto & ChildValue : Value )
		{
			ProcessFiltersOnJson( Params, ChildValue );
		}

		// Process.
		if( Value.type() == Json::objectValue )
		{
			std::vector< std::string > FilterGroups;
			std::cmatch Match;

			// Gather filter groups.
			auto MemberNames = Value.getMemberNames();
			for( const auto& MemberName : MemberNames )
			{
				if( std::regex_match( MemberName.c_str(), Match, GRegex_Filter ) )
				{
					FilterGroups.push_back( MemberName );
				}
			}

			// Sort filter groups alphabetically.
			std::sort( FilterGroups.begin(), FilterGroups.end() );

			// Move contents of each filter group that we should.
			for( const auto& FilterGroup : FilterGroups )
			{
				if( Params.checkFilterString( FilterGroup ) )
				{
					const auto& Group = Value[ FilterGroup ];
					auto GroupMemberNames = Group.getMemberNames();
					for( const auto& GroupMemberName : GroupMemberNames )
					{
						Value[ GroupMemberName ] = Group[ GroupMemberName ];
					}
				}

				// Remove filter group.
				Value.removeMember( FilterGroup );
			}
		}
	}
}

#if PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPackageImporter::CsPackageImporter(
		const CsPlatformParams& Params,
		const BcName& Name,
		const BcPath& Filename ):
	Params_( Params ),
	Name_( Name ),
	Filename_( Filename ),
	Resources_(),
	BuildingBeginCount_( 0 ),
	ImportErrorCount_( 0 ),
	ResourceIds_( 0 ),
	DataPosition_( 0 )
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
BcBool CsPackageImporter::import()
{
	PSY_LOGSCOPEDCATEGORY( "Import" );
	PSY_LOG( "Importing %s...\n", Filename_.c_str() );

	PSY_LOGSCOPEDINDENT;

	BcTimer TotalTimer;
	TotalTimer.mark();

	// Store source file info.
	FsStats Stats;
	if( FsCore::pImpl()->fileStats( Filename_.c_str(), Stats ) )
	{
		Header_.SourceFileStatsHash_ = BcHash( reinterpret_cast< BcU8* >( &Stats ), sizeof( Stats ) );
	}
	else
	{
		Header_.SourceFileStatsHash_ = 0;
	}

	beginImport();
	Header_.SourceFile_ = addString( Filename_.c_str() );
	endImport();

	Json::Value Root;
	if( loadJsonFile( Filename_.c_str(), Root ) )
	{
		// Process filters.
		ProcessFiltersOnJson( Params_, Root );

		// Add as dependency.
		beginImport();
		addDependency( Filename_.c_str() );

		// Get resource list.
		Json::Value Resources( Root.get( "resources", Json::Value( Json::arrayValue ) ) );

		// Add all package cross refs.
		addAllPackageCrossRefs( Resources );

		// Set resource id to zero.
		ResourceIds_.store( 0 );

		// Import everything.
		for( const auto& ResourceObject : Resources )
		{
			addImport( ResourceObject, BcFalse );
		}
		endImport();

		// Sort importers.
		std::sort( Resources_.begin(), Resources_.end() );

		// Iterate over all resources and import (import calls can append to the list)
		size_t CurrResourceIdx = 0;
		while( CurrResourceIdx < Resources_.size() )
		{
			// Grab first resource in the list.
			auto ResourceEntry = std::move( Resources_[ CurrResourceIdx++ ] );
			
			// Import resource.
			BcTimer ResourceTimer;
			ResourceTimer.mark();
			try
			{
				PSY_LOGSCOPEDINDENT;
				beginImport();

				if( importResource( 
					std::move( ResourceEntry.Importer_ ), 
					ResourceEntry.Resource_ ) )
				{
					PSY_LOG( "SUCCEEDED: Time: %.2f seconds.\n", ResourceTimer.time() );
				}
				else
				{
					PSY_LOG( "FAILED: Time: %.2f seconds.\n", ResourceTimer.time() );
					endImport();
					return BcFalse;
				}

				endImport();
			}
			catch( CsImportException ImportException )
			{
				PSY_LOG( "FAILED: Time: %.2f seconds.\n", ResourceTimer.time() );
				PSY_LOG( "ERROR: in file %s:\n%s\n", ImportException.file(), ImportException.what() );	
				endImport();
				return BcFalse;
			}
		}

		// Save and return.
		BcPath PackedPackage( Params_.getPackagePackedPath( Name_ ) );
		BcBool SaveSuccess = save( PackedPackage );

		if( SaveSuccess )
		{
			PSY_LOG( "SUCCEEDED: Time: %.2f seconds.\n", TotalTimer.time() );

			// Write out dependencies.
			std::string OutputDependencies = *Params_.getPackageIntermediatePath( Name_ ) + "/deps.json";
			CsSerialiserPackageObjectCodec ObjectCodec( nullptr, (BcU32)bcRFF_ALL, (BcU32)bcRFF_TRANSIENT, 0 );
			SeJsonWriter Writer( &ObjectCodec );
			Writer << Dependencies_;
			Writer.save( OutputDependencies.c_str() );
		}
		else
		{
			PSY_LOG( "FAILED: Time: %.2f seconds.\n", TotalTimer.time() );
		}

		return SaveSuccess;
	}
	
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// save
BcBool CsPackageImporter::save( const BcPath& Path )
{
	// Create target folder.
	const auto& PackedPath = Params_.PackedContentPath_;

	if( !BcFileSystemExists( PackedPath.c_str() ) )
	{
		BcFileSystemCreateDirectories( PackedPath.c_str() );
	}

	// Open package output.
	BcPath TempFile = *Path + ".tmp";
	if( File_.open( (*TempFile).c_str(), bcFM_WRITE ) )
	{
		// Generate string table.
		BcStream StringTableStream;

		for( BcU32 Idx = 0; Idx < StringList_.size(); ++Idx )
		{
			const std::string& StringEntry( StringList_[ Idx ] );
			StringTableStream.push( StringEntry.c_str(), StringEntry.size() );
			StringTableStream << '\0';
		}	

		// Setup header.
		Header_.Magic_ = CsPackageHeader::MAGIC;
		Header_.Version_ = CsPackageHeader::VERSION;
		Header_.Flags_ = csPF_DEFAULT; // TODO: Flags.
		Header_.StringTableBytes_ = (BcU32)StringTableStream.dataSize();
		Header_.TotalPackageCrossRefs_ = (BcU32)PackageCrossRefList_.size();
		Header_.TotalPackageDependencies_ = (BcU32)PackageDependencyList_.size();
		Header_.TotalResources_ = (BcU32)ResourceHeaders_.size();
		Header_.TotalChunks_ = (BcU32)ChunkHeaders_.size();
		Header_.TotalAllocSize_ = 0;
		Header_.MinAlignment_ = 16;		// TODO: Platform specific.
		Header_.MaxAlignment_ = 4096;	// TODO: Platform specific.

		// Calculate package alloc size.
		Header_.TotalAllocSize_ += (BcU32)StringTableStream.dataSize();
		Header_.TotalAllocSize_ += (BcU32)BcCalcAlignment( (BcU32)PackageCrossRefList_.size() * (BcU32)sizeof( CsPackageCrossRefData ), Header_.MinAlignment_ );
		Header_.TotalAllocSize_ += (BcU32)BcCalcAlignment( (BcU32)PackageDependencyList_.size() * (BcU32)sizeof( CsPackageDependencyData ), Header_.MinAlignment_ );
		Header_.TotalAllocSize_ += (BcU32)BcCalcAlignment( (BcU32)ResourceHeaders_.size() * (BcU32)sizeof( CsPackageResourceHeader ), Header_.MinAlignment_ );
		Header_.TotalAllocSize_ += (BcU32)BcCalcAlignment( (BcU32)ChunkHeaders_.size() * (BcU32)sizeof( CsPackageChunkHeader ), Header_.MinAlignment_ );
		Header_.TotalAllocSize_ += (BcU32)BcCalcAlignment( (BcU32)ChunkHeaders_.size() * (BcU32)sizeof( CsPackageChunkData ), Header_.MinAlignment_ );
		
		// Align total size to 1 page for the start of resource data.
		Header_.TotalAllocSize_ = (BcU32)BcCalcAlignment( Header_.TotalAllocSize_, Header_.MaxAlignment_ );

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

		// Write package cross refs.
		for( BcU32 Idx = 0; Idx < PackageCrossRefList_.size(); ++Idx )
		{
			CsPackageCrossRefData& CrossRefData = PackageCrossRefList_[ Idx ];
			File_.write( &CrossRefData, sizeof( CsPackageCrossRefData ) );
		}

		// Write package dependencies.
		for( BcU32 Idx = 0; Idx < PackageDependencyDataList_.size(); ++Idx )
		{
			CsPackageDependencyData& PackageDependencyData = PackageDependencyDataList_[ Idx ];
			File_.write( &PackageDependencyData, sizeof( CsPackageDependencyData ) );
		}

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

		// Rename.
		if( BcFileSystemExists( (*Path).c_str() ) )
		{
			BcFileSystemRemove( (*Path).c_str() );
		}
		BcFileSystemRename( (*TempFile).c_str(), (*Path).c_str() );

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
		auto Data = File.readAllBytes();		
		Json::Reader Reader;
		
		if( Reader.parse( (const char*)Data.get(), (const char*)Data.get() + File.size(), Root ) )
		{
			Success = BcTrue;
		}
		else
		{
 			throw CsImportException( pFileName, "%s", Reader.getFormattedErrorMessages().c_str() );
		}
	}
	else
	{
		throw CsImportException( pFileName, "Failed to load Json." );
	}
	
	return Success;
}

//////////////////////////////////////////////////////////////////////////
// importResource
BcBool CsPackageImporter::importResource( 
	CsResourceImporterUPtr Importer, 
	const Json::Value& Resource )
{
	// Catch name being missing.
	if( Importer->getResourceName().empty() )
	{
		PSY_LOG( "ERROR: Name not specified for resource.\n" );
		return BcFalse;
	}

	// Catch type being missing.
	if( Importer->getResourceType().empty() )
	{
		PSY_LOG( "ERROR: Type not specified for resource.\n" );
		return BcFalse;
	}
	
	PSY_LOG( "INFO: Processing \"%s\" of type \"%s\"\n", 
		Importer->getResourceName().c_str(), Importer->getResourceType().c_str() );
	
	// Get first chunk used by resource.
	size_t FirstChunk = ChunkHeaders_.size();

	BcBool SuccessfulImport = BcFalse;

	// NOTE: Eventually we will be exception safe throught the import
	//       pipeline, so shouldn't need these adhoc try/catch blocks.
	try
	{
		PSY_LOGSCOPEDINDENT;
		SuccessfulImport = Importer->import();

		// Check for error + critical messages.
		SuccessfulImport &= Importer->getMessageCount( CsMessageCategory::ERROR ) == 0;
		SuccessfulImport &= Importer->getMessageCount( CsMessageCategory::CRITICAL ) == 0;
	}
	catch( CsImportException ImportException )
	{
		PSY_LOG( "ERROR: %s", ImportException.what() );
	}
	
	// Handle success.
	if( SuccessfulImport )
	{
		// Setup current resource header.
		CurrResourceHeader_.Name_ = addString( Importer->getResourceName().c_str() );
		CurrResourceHeader_.Type_ = addString( Importer->getResourceType().c_str() );
		CurrResourceHeader_.Flags_ = csPEF_DEFAULT;
		CurrResourceHeader_.FirstChunk_ = static_cast< BcU32 >( FirstChunk );
		CurrResourceHeader_.LastChunk_ = static_cast< BcU32 >( ChunkHeaders_.size() - 1 ); // Assumes 1 chunk for resource. Fair assumption.
		
		// Make sure chunk indices are valid.
		BcAssert( CurrResourceHeader_.FirstChunk_ <= CurrResourceHeader_.LastChunk_ );

		ResourceHeaders_.push_back( CurrResourceHeader_ );
	}
	
	return SuccessfulImport;
}

//////////////////////////////////////////////////////////////////////////
// getName
BcName CsPackageImporter::getName() const
{
	return Name_;
}

//////////////////////////////////////////////////////////////////////////
// beginImport
void CsPackageImporter::beginImport()
{
	BuildingBeginCount_++;
}

//////////////////////////////////////////////////////////////////////////
// endImport
void CsPackageImporter::endImport()
{
	BuildingBeginCount_--;
}

//////////////////////////////////////////////////////////////////////////
// addImport
BcU32 CsPackageImporter::addImport( const Json::Value& Resource, BcBool IsCrossRef )
{
	PSY_LOGSCOPEDINDENT;

	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );

	// Validate it's an object.
	BcAssertMsg( Resource.type() == Json::objectValue, "Can't import a value that isn't an object." );

	// Validate name and type.
	Json::Value Name( Resource.get( "name", Json::nullValue ) );
	Json::Value Type( Resource.get( "$Class", Json::nullValue ) );
	if( Type == Json::nullValue )
	{
		Type = Resource.get( "type", Json::nullValue );
		BcAssert( Type != Json::nullValue );
	}
	BcAssertMsg( Name.type() == Json::stringValue, "Name not specified for resource.\n" );
	BcAssertMsg( Type.type() == Json::stringValue, "Type not specified for resource.\n" )

	// Check if there is a resource with matching name already, as long as it isn't a cross ref.
	if( !IsCrossRef )
	{
		auto AlreadyExisting = std::find_if( Resources_.begin(), Resources_.end(),
			[ this, &Name ]( const TResourceImport& ResourceImport )
			{
				return ResourceImport.Importer_->getResourceName() == Name.asCString();
			} );

		BcAssertMsg( AlreadyExisting == Resources_.end(),
			"Resource \"%s\" already exists in package \"%s\"",
			Name.asCString(),
			(*Name_).c_str() );
	}

	// Grab class, create importer.
	const ReClass* ResourceClass = ReManager::GetClass( Type.asCString() );
	CsResourceImporterUPtr ResourceImporter;
	if( ResourceClass != nullptr )
	{
		CsResourceImporterAttribute* ResourceImporterAttr = nullptr;
		do
		{
			ResourceImporterAttr =
				ResourceClass->getAttribute< CsResourceImporterAttribute >();

			// Check on a parent to see if there is a valid importer attached to it.
			if( ResourceImporterAttr == nullptr )
			{
				ResourceClass = ResourceClass->getSuper();
			}
		}
		while( ResourceImporterAttr == nullptr && ResourceClass != nullptr );

		if( ResourceImporterAttr != nullptr )
		{
			ResourceImporter = ResourceImporterAttr->getImporter();
		}
		
		// Add dependencies.
		addDependency( ResourceClass );
		addDependency( ResourceImporter->getClass() );
	}

	// 
	BcAssertMsg( ResourceImporter != nullptr, 
		"Can't create resource importer for \"%s\"", Type.asCString() );

	// Serialise resource onto importer.
	CsSerialiserPackageObjectCodec ObjectCodec( nullptr, bcRFF_IMPORTER, bcRFF_NONE, bcRFF_IMPORTER );
	SeJsonReader Reader( &ObjectCodec );
	Reader.setMemberMismatchIsError( true );
	Reader.serialiseClassMembers( ResourceImporter.get(), ResourceImporter->getClass(), Resource, 0 );

	// Check for mismatch member errors.
	if( Reader.getNoofMemberMismatchErrors() > 0 )
	{
		throw CsImportException( "", "Member mismatch errors. See above." );
	}

	// Add import with importer.
	return addImport( std::move( ResourceImporter ), Resource, IsCrossRef );
}

//////////////////////////////////////////////////////////////////////////
// addImport
BcU32 CsPackageImporter::addImport( 
	CsResourceImporterUPtr Importer,
	const Json::Value& Resource, 
	BcBool IsCrossRef )
{
	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );

	// Get id.
	BcU32 ResourceId = ResourceIds_.fetch_add( 1 );

	// Initialise importer.
	Importer->initialise( this, ResourceId );

	// Cache name and type.
	const auto ResourceName = Importer->getResourceName();
	const auto ResourceType = Importer->getResourceType();
	
	// Push into resource list.
	TResourceImport ResourceImport;
	ResourceImport.Importer_ = std::move( Importer );
	ResourceImport.Resource_ = Resource;
	Resources_.push_back( std::move( ResourceImport ) );
	
	// If it's a cross ref, do that shiz.
	if( IsCrossRef )
	{
		// Construct and add package cross ref.
		std::string CrossRef;
		CrossRef += "$(";
		CrossRef += std::to_string( ResourceId );
		CrossRef += ")";
		
		return addPackageCrossRef( CrossRef.c_str() );
	}

	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// addString
BcU32 CsPackageImporter::addString( const BcChar* pString )
{
	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );
	BcAssert( pString != nullptr );
	
	BcU32 CurrentOffset = 0;

	for( BcU32 Idx = 0; Idx < StringList_.size(); ++Idx )
	{
		const std::string& StringEntry( StringList_[ Idx ] );

		if( StringEntry == pString )
		{
			return CurrentOffset;
		}

		// String length with null terminator.
		CurrentOffset += static_cast< BcU32 >( StringEntry.length() + 1 );
	}

	// Add string to list.
	StringList_.push_back( pString );

	// Return current offset.
	return CurrentOffset;
}

//////////////////////////////////////////////////////////////////////////
// addPackageCrossRef
BcU32 CsPackageImporter::addPackageCrossRef( const BcChar* pFullName )
{
	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );

	BcBool IsWeak = BcFalse;
	std::cmatch Match;
	std::regex_match( pFullName, Match, GRegex_ResourceReference );

	// Try the weak match.
	// TODO: Merge into  regex.
	if( Match.size() == 0 )
	{
		IsWeak = BcTrue;
		std::regex_match( pFullName, Match, GRegex_WeakResourceReference );
	}

	if( Match.size() == 4 )
	{	
		// Match against normal refs.
		std::string TypeName;
		std::string PackageName;
		std::string ResourceName;

		TypeName = Match[ 1 ];
		PackageName = Match[ 2 ];
		ResourceName = Match[ 3 ];

		// Handle "this" for package.
		if( PackageName == "this" )
		{
			PackageName = *Name_;
		}
	
		// Add cross ref.
		CsPackageCrossRefData CrossRef = 
		{
			addString( TypeName.c_str() ),
			addString( PackageName.c_str() ),
			addString( ResourceName.c_str() ),
			IsWeak,
			BcFalse
		};

		// Add if it doesn't exist already.
		BcU32 FoundIdx = BcErrorCode;
		for( BcU32 Idx = 0; Idx < PackageCrossRefList_.size(); ++Idx )
		{
			if( PackageCrossRefList_[ Idx ] == CrossRef )
			{
				FoundIdx = Idx;
				break;
			}
		}

		if( FoundIdx == BcErrorCode )
		{
			PackageCrossRefList_.push_back( CrossRef );
			FoundIdx = static_cast< BcU32 >( PackageCrossRefList_.size() - 1 );
		}

		// Add package dependency if it's not there, and it's not this package.
		if( PackageName != *Name_ && havePackageDependency( PackageName ) == BcFalse )
		{
			CsPackageDependencyData PackageDependency =
			{
				CrossRef.PackageName_,
				IsWeak
			};
			
			PackageDependencyDataList_.push_back( PackageDependency );
			PackageDependencyList_.push_back( PackageName );
		}

		// Resource index to the package cross ref.
		return FoundIdx;
	}
	
	// Try ID match.
	if( Match.size() == 0 )
	{
		std::regex_match( pFullName, Match, GRegex_ResourceIdReference );
	}
	if( Match.size() == 2 )
	{
		// Match against normal refs.
		std::string ResourceId;

		ResourceId = Match[ 1 ];
	
		// Add cross ref.
		CsPackageCrossRefData CrossRef = 
		{
			0,
			0,
			static_cast< BcU32 >( std::stoi( ResourceId ) ),
			BcFalse,
			BcTrue
		};

		// Add if it doesn't exist already.
		BcU32 FoundIdx = BcErrorCode;
		for( BcU32 Idx = 0; Idx < PackageCrossRefList_.size(); ++Idx )
		{
			if( PackageCrossRefList_[ Idx ] == CrossRef )
			{
				FoundIdx = Idx;
				break;
			}
		}

		if( FoundIdx == BcErrorCode )
		{
			PackageCrossRefList_.push_back( CrossRef );
			FoundIdx = static_cast< BcU32 >( PackageCrossRefList_.size() - 1 );

			PSY_LOG( "Adding crossref %u: %s\n", FoundIdx, pFullName );
		}

		// Resource index to the package cross ref.
		return FoundIdx;
	}

	BcAssertMsg( BcFalse, "Cross package ref \"%s\" is not formatted correctly.", pFullName );
	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// addChunk
BcU32 CsPackageImporter::addChunk( BcU32 ID, const void* pData, BcSize Size, BcSize RequiredAlignment, BcU32 Flags )
{
	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );

	BcAssert( Size > 0 );
	BcAssert( BcPot( (BcU32)RequiredAlignment ) );
	BcAssert( RequiredAlignment <= 4096 );

	const BcU8* pPackedData = reinterpret_cast< const BcU8* >( pData );
	size_t PackedSize = Size;
	BcBool HaveCompressed = BcFalse;

	// If we need to compress, do so.
	if( ( Flags & csPCF_COMPRESSED ) != 0 )
	{
		if( BcCompressData( static_cast< const BcU8* >( pData ), Size, pPackedData, PackedSize ) )
		{
			HaveCompressed = BcTrue;
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
	ChunkHeader.RequiredAlignment_ = static_cast< BcU32 >( RequiredAlignment );
	ChunkHeader.PackedBytes_ = static_cast< BcU32 >( PackedSize );
	ChunkHeader.UnpackedBytes_ = static_cast< BcU32 >( Size );
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
	if( HaveCompressed )
	{
		delete [] pPackedData;
	}

	// Push into lists.
	ChunkHeaders_.push_back( ChunkHeader );
	ChunkDatas_.push_back( ChunkData );

	return static_cast< BcU32 >( ChunkHeaders_.size() - 1 );
}

//////////////////////////////////////////////////////////////////////////
// addDependency
void CsPackageImporter::addDependency( const BcChar* pFileName )
{
	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );
	Dependencies_.Dependencies_.insert( CsDependency( pFileName ) );
}

//////////////////////////////////////////////////////////////////////////
// addDependency
void CsPackageImporter::addDependency( const ReClass* Class )
{
	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );
	Dependencies_.addClass( Class );
}


//////////////////////////////////////////////////////////////////////////
// addAllPackageCrossRefs
void CsPackageImporter::addAllPackageCrossRefs( Json::Value& Root )
{
	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );

	// If it's a string value, attempt to match it.
	if( Root.type() == Json::stringValue )
	{
		std::cmatch Match;
		std::regex_match( Root.asCString(), Match, GRegex_ResourceReference );
		
		// Try the weak match.
		// TODO: Merge into  regex.
		if( Match.size() == 0 )
		{
			std::regex_match( Root.asCString(), Match, GRegex_WeakResourceReference );
		}

		if( Match.size() == 4 )
		{
			BcU32 RefIndex = addPackageCrossRef( Root.asCString() );

			// If we find it, replace string reference with a cross ref index.
			if( RefIndex != BcErrorCode )
			{
				Root = Json::Value( RefIndex );
			}
		}
	}
	else if( Root.type() == Json::arrayValue )
	{
		for( BcU32 Idx = 0; Idx < Root.size(); ++Idx )
		{
			addAllPackageCrossRefs( Root[ Idx ] );
		}
	}
	else if( Root.type() == Json::objectValue )
	{
		Json::Value::Members MemberValues = Root.getMemberNames();

		for( BcU32 Idx = 0; Idx < MemberValues.size(); ++Idx )
		{
			addAllPackageCrossRefs( Root[ MemberValues[ Idx ] ] );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getParams
const CsPlatformParams& CsPackageImporter::getParams() const
{
	return Params_;
}

//////////////////////////////////////////////////////////////////////////
// havePackageDependency
BcBool CsPackageImporter::havePackageDependency( const BcName& PackageName )
{
	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );

	for( TPackageDependencyIterator It( PackageDependencyList_.begin() ); It != PackageDependencyList_.end(); ++It )
	{
		if( PackageName == (*It) )
		{
			return BcTrue;
		}
	}
	
	return BcFalse;
}

#endif // PSY_IMPORT_PIPELINE
