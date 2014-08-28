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

#include "Reflection/ReReflection.h"
#include "Serialisation/SeJsonWriter.h"

#include "Base/BcStream.h"
#include "Base/BcCompression.h"

#include "Base/BcRegex.h"
#include "Base/BcMath.h"

#include "System/SysKernel.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

//////////////////////////////////////////////////////////////////////////
// Regex for resource references.
BcRegex GRegex_ResourceReference( "^\\$\\((.*?):(.*?)\\.(.*?)\\)" );		// Matches "$(Type:Package.Resource)"
BcRegex GRegex_WeakResourceReference( "^\\#\\((.*?):(.*?)\\.(.*?)\\)" );		// Matches "#(Type:Package.Resource)" // TODO: Merge into the ResourceReference regex.

#if PSY_SERVER

//////////////////////////////////////////////////////////////////////////
// CsPackageDependencies
REFLECTION_DEFINE_BASIC( CsPackageDependencies );

void CsPackageDependencies::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Dependencies_",	&CsPackageDependencies::Dependencies_ ),
	};

	ReRegisterClass< CsPackageDependencies >( Fields );
};

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
	Name_ = Name;
	BcPath Path = CsCore::pImpl()->getPackageImportPath( Name );

	BcPrintf( "============================================================================\n" );
	BcPrintf( "CsPackageImporter: Importing %s...\n", (*Path).c_str() );

	BcTimer TotalTimer;
	TotalTimer.mark();

	// Store source file info.
	FsStats Stats;
	if( FsCore::pImpl()->fileStats( (*Path).c_str(), Stats ) )
	{
		Header_.SourceFileStatsHash_ = BcHash( reinterpret_cast< BcU8* >( &Stats ), sizeof( Stats ) );
	}
	else
	{
		Header_.SourceFileStatsHash_ = 0;
	}

	beginImport();
	Header_.SourceFile_ = addString( (*Path).c_str() );
	endImport();

	Json::Value Root;
	if( loadJsonFile( (*Path).c_str(), Root ) )
	{
		// Add as dependency.
		beginImport();
		addDependency( (*Path).c_str() );

		// Get resource list.
		Json::Value Resources( Root.get( "resources", Json::Value( Json::arrayValue ) ) );

		// Add all package cross refs.
		addAllPackageCrossRefs( Resources );
		endImport();

#if THREADED_IMPORTING
		// Import resource.
		BcTimer ResourceTimer;
		ResourceTimer.mark();

		// Import everything.
		for( const auto& ResourceObject : Resources )
		{
			addImport( ResourceObject, BcFalse );
		}

		// Wait for fence.
		BuildingFence_.wait();

		// Check import error count for failures.
		if( ImportErrorCount_ > 0 )
		{
			BcPrintf( "FAILED IMPORTING. Time: %.2f seconds.\n", ResourceTimer.time() );
			BcBreakpoint;
			return BcFalse;
		}
#else
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
			BcTimer ResourceTimer;
			ResourceTimer.mark();
			try
			{
				beginImport();

				if( importResource( ResourceObject ) )
				{
					BcPrintf( " - - SUCCEEDED. Time: %.2f seconds.\n", ResourceTimer.time() );
				}
				else
				{
					BcPrintf( " - - FAILED. Time: %.2f seconds.\n", ResourceTimer.time() );
 					BcBreakpoint;
					endImport();
					return BcFalse;
				}

				endImport();
			}
			catch( CsImportException ImportException )
			{
				BcPrintf( " - - FAILED. Time: %.2f seconds.\n", ResourceTimer.time() );
				BcPrintf( "CsPackageImporter: Import error in file %s:\n%s\n", ImportException.file().c_str(), ImportException.what() );	
				endImport();
				return BcFalse;
			}
		}
#endif

		// Save and return.
		BcPath PackedPackage( CsCore::pImpl()->getPackagePackedPath( Name ) );
		BcBool SaveSuccess = save( PackedPackage );

		if( SaveSuccess )
		{
			BcPrintf( " SUCCEEDED. Time: %.2f seconds.\n", TotalTimer.time() );

			// Write out dependencies.
			std::string OutputDependencies = *CsCore::pImpl()->getPackageIntermediatePath( Name ) + "/deps.json";
			CsSerialiserPackageObjectCodec ObjectCodec( nullptr );
			SeJsonWriter Writer( &ObjectCodec );
			Writer << Dependencies_;
			Writer.save( OutputDependencies.c_str() );
		}
		else
		{
			BcPrintf( " FAILED. Time: %.2f seconds.\n", TotalTimer.time() );
			BcBreakpoint;
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
	std::string PackedPath = *CsCore::pImpl()->getPackagePackedPath( "" );
	if( !boost::filesystem::exists( PackedPath ) )
	{
		boost::filesystem::create_directories( PackedPath );
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
			StringTableStream.push( StringEntry.c_str(), StringEntry.size() + 1 );
		}	

		// Setup header.
		Header_.Magic_ = CsPackageHeader::MAGIC;
		Header_.Version_ = CsPackageHeader::VERSION;
		Header_.Flags_ = csPF_DEFAULT; // TODO: Flags.
		Header_.StringTableBytes_ = StringTableStream.dataSize();
		Header_.TotalPackageCrossRefs_ = (BcU32)PackageCrossRefList_.size();
		Header_.TotalPackageDependencies_ = (BcU32)PackageDependencyList_.size();
		Header_.TotalResources_ = (BcU32)ResourceHeaders_.size();
		Header_.TotalChunks_ = (BcU32)ChunkHeaders_.size();
		Header_.TotalAllocSize_ = 0;
		Header_.MinAlignment_ = 16;		// TODO: Platform specific.
		Header_.MaxAlignment_ = 4096;	// TODO: Platform specific.

		// Calculate package alloc size.
		Header_.TotalAllocSize_ += StringTableStream.dataSize();
		Header_.TotalAllocSize_ += BcCalcAlignment( PackageCrossRefList_.size() * sizeof( CsPackageCrossRefData ), Header_.MinAlignment_ );
		Header_.TotalAllocSize_ += BcCalcAlignment( PackageDependencyList_.size() * sizeof( CsPackageDependencyData ), Header_.MinAlignment_ );
		Header_.TotalAllocSize_ += BcCalcAlignment( ResourceHeaders_.size() * sizeof( CsPackageResourceHeader ), Header_.MinAlignment_ );
		Header_.TotalAllocSize_ += BcCalcAlignment( ChunkHeaders_.size() * sizeof( CsPackageChunkHeader ), Header_.MinAlignment_ );
		Header_.TotalAllocSize_ += BcCalcAlignment( ChunkHeaders_.size() * sizeof( CsPackageChunkData ), Header_.MinAlignment_ );
		
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
		if( boost::filesystem::exists( *Path ) )
		{
			boost::filesystem::remove( *Path );
		}
		boost::filesystem::rename( *TempFile, *Path );

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
		const BcU8* pData = File.readAllBytes();		
		Json::Reader Reader;
		
		if( Reader.parse( (const char*)pData, (const char*)pData + File.size(), Root ) )
		{
			Success = BcTrue;
		}
		else
		{
			BcPrintf( "CsPackageImporter: Failed to parse Json:\n %s\n", Reader.getFormatedErrorMessages().c_str() );
 			BcAssertMsg( BcFalse, "Failed to parse \"%s\", see log for more details.", pFileName );
		}
		
		BcMemFree( (void*)pData );
	}
	else
	{
		BcAssertMsg( BcFalse, "Failed to load \"%s\"", pFileName );
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

	// Find class and attribute.
	BcBool SuccessfulImport = BcFalse;
	const ReClass* ResourceClass = ReManager::GetClass( Type.asCString() );
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
			auto ResourceImporter = ResourceImporterAttr->getImporter();

			// TODO: Use serialisation to setup resource importer.
			try
			{
				ResourceImporter->initialise( this );
				SuccessfulImport = ResourceImporter->import( Resource );
			}
			catch( CsImportException ImportException )
			{
				throw ImportException;
			}
		}
	}
	
	// Fallback to old import.
	// TODO: No more old import pipeline.
	if( SuccessfulImport == BcFalse )
	{
		BcBreakpoint;
	}

	// Handle success.
	if( SuccessfulImport )
	{
		// Setup current resource header.
		CurrResourceHeader_.Name_ = addString( Name.asCString() );
		CurrResourceHeader_.Type_ = addString( Type.asCString() );
		CurrResourceHeader_.Flags_ = csPEF_DEFAULT;
		CurrResourceHeader_.FirstChunk_ = FirstChunk;
		CurrResourceHeader_.LastChunk_ = ChunkHeaders_.size() - 1; // Assumes 1 chunk for resource. Fair assumption.
		
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
// importResource_worker
BcBool CsPackageImporter::importResource_worker( Json::Value ResourceObject )
{
	BcTimer ResourceTimer;
	ResourceTimer.mark();
	
	try
	{
		if( importResource( ResourceObject ) )
		{
			BcPrintf( " - - SUCCEEDED. Time: %.2f seconds.\n", ResourceTimer.time() );
		}
		else
		{
			BcPrintf( " - - FAILED. Time: %.2f seconds.\n", ResourceTimer.time() );
 			BcBreakpoint;
			return BcFalse;
		}
	}
	catch( CsImportException ImportException )
	{
		BcPrintf( " - - FAILED. Time: %.2f seconds.\n", ResourceTimer.time() );
		BcPrintf( "CsPackageImporter: Import error in file %s:\n%s\n", ImportException.file().c_str(), ImportException.what() );	
		return BcFalse;
	}

	BuildingFence_.decrement();
	return BcTrue;
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
	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );

	// Validate it's an object.
	BcAssertMsg( Resource.type() == Json::objectValue, "CsPackageImporter: Can't import a value that isn't an object." );

	// Validate name and type.
	Json::Value Name( Resource.get( "name", Json::Value( Json::nullValue ) ) );
	Json::Value Type( Resource.get( "type", Json::Value( Json::nullValue ) ) );
	BcAssertMsg( Name.type() == Json::stringValue, "CsPackageImporter: Name not specified for resource.\n" );
	BcAssertMsg( Type.type() == Json::stringValue, "CsPackageImporter: Type not specified for resource.\n" )

	// Handle crossrefs slightly differently.
	if( IsCrossRef )
	{
		// Prefix crossrefs so they don't conflict.
		Name = boost::str( boost::format( "XREF%1%%2%" ) % PackageCrossRefList_.size() % Name.asCString() );
		Json::Value NewResource = Resource;
		NewResource[ "name" ] = Name;

		// Put to front of list so it's imported next.
		JsonResources_.push_back( NewResource );
	}
	else
	{
		// Put to front of list so it's imported next.
		JsonResources_.push_back( Resource );
	}

#if THREADED_IMPORTING
	// Increment fence so we know to wait.
	BuildingFence_.increment();

	// Push job.
	BcDelegate< BcBool(*)( Json::Value ) > Delegate( BcDelegate< BcBool(*)( Json::Value ) >::bind< CsPackageImporter, &CsPackageImporter::importResource_worker >( this ) );
	SysKernel::pImpl()->pushDelegateJob( 0, Delegate, Resource );
#endif

	// If it's a cross ref, do that shiz.
	if( IsCrossRef )
	{
		// Construct and add package cross ref.
		std::string CrossRef;
		CrossRef += "$(";
		CrossRef += Type.asString();
		CrossRef += ":";
		CrossRef += *Name_;
		CrossRef += ".";
		CrossRef += Name.asString();
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
// addPackageCrossRef
BcU32 CsPackageImporter::addPackageCrossRef( const BcChar* pFullName )
{
	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );

	BcBool IsWeak = BcFalse;
	BcRegexMatch Match;
	BcU32 Matches = GRegex_ResourceReference.match( pFullName, Match );

	// Try the weak match.
	// TODO: Merge into  regex.
	if( Matches == 0 )
	{
		IsWeak = BcTrue;
		Matches = GRegex_WeakResourceReference.match( pFullName, Match );
	}

	if( Matches == 4 )
	{	
		std::string TypeName;
		std::string PackageName;
		std::string ResourceName;

		Match.getMatch( 1, TypeName );
		Match.getMatch( 2, PackageName );
		Match.getMatch( 3, ResourceName );

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
			IsWeak
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
			FoundIdx = PackageCrossRefList_.size() - 1;
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
	else
	{
		BcAssertMsg( BcFalse, "Cross package ref \"%s\" is not formatted correctly.", pFullName );
	}

	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// addChunk
BcU32 CsPackageImporter::addChunk( BcU32 ID, const void* pData, BcSize Size, BcSize RequiredAlignment, BcU32 Flags )
{
	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );

	BcAssert( Size > 0 );
	BcAssert( BcPot( RequiredAlignment ) );
	BcAssert( RequiredAlignment <= 4096 );

	const BcU8* pPackedData = reinterpret_cast< const BcU8* >( pData );
	BcU32 PackedSize = Size;

	// If we need to compress, do so.
	if( ( Flags & csPCF_COMPRESSED ) != 0 )
	{
		if( BcCompressData( static_cast< const BcU8* >( pData ), Size, pPackedData, PackedSize ) )
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
	std::lock_guard< std::recursive_mutex > Lock( BuildingLock_ );
	BcAssert( BuildingBeginCount_ > 0 );
	Dependencies_.Dependencies_.insert( CsDependency( pFileName ) );
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
		BcRegexMatch Match;
		BcU32 Matches = GRegex_ResourceReference.match( Root.asCString(), Match );
		
		// Try the weak match.
		// TODO: Merge into  regex.
		if( Matches == 0 )
		{
			Matches = GRegex_WeakResourceReference.match( Root.asCString(), Match );
		}

		if( Matches == 4 )
		{
			BcU32 RefIndex = addPackageCrossRef( Root.asCString() );

			// If we find it, replace string reference with a cross ref index.
			if( RefIndex != BcErrorCode )
			{
				BcPrintf("- Adding crossref %u: %s\n", RefIndex, Root.asCString() );
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

#endif

