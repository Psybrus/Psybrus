/**************************************************************************
*
* File:		CsResourceImporter.cpp
* Author:	Neil Richardson
* Ver/Date:	
* Description:
*
*
*
*
*
**************************************************************************/

#include "System/Content/CsResourceImporter.h"
#include "System/Content/CsPackageImporter.h"
#include "System/Content/CsCore.h"

#if PSY_IMPORT_PIPELINE
#include <boost/uuid/sha1.hpp>
#include <json/json.h>
#endif // PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// Attribute
REFLECTION_DEFINE_DERIVED( CsResourceImporterAttribute );

void CsResourceImporterAttribute::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "ImporterClass_", &CsResourceImporterAttribute::ImporterClass_ ),
	};
		
	ReRegisterClass< CsResourceImporterAttribute, Super >( Fields );
}


//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_BASE( CsResourceImporter );

void CsResourceImporter::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Name_", &CsResourceImporter::Name_, bcRFF_IMPORTER ),
		new ReField( "Type_", &CsResourceImporter::Type_, bcRFF_IMPORTER ),
		new ReField( "MessageCount_", &CsResourceImporter::MessageCount_ ),
		new ReField( "Importer_", &CsResourceImporter::Importer_, bcRFF_TRANSIENT ),
	};
	
	ReRegisterClass< CsResourceImporter >( Fields );
}

CsResourceImporterAttribute::CsResourceImporterAttribute():
	ImporterClass_( nullptr ),
	VersionId_( 0 )
{

}

CsResourceImporterAttribute::CsResourceImporterAttribute( 
		const ReClass* ImporterClass,
		BcU32 VersionId,
		BcU32 Priority ):
	ImporterClass_( ImporterClass ),
	VersionId_( VersionId ),
	Priority_( Priority )
{

}

CsResourceImporterUPtr CsResourceImporterAttribute::getImporter() const
{
	return CsResourceImporterUPtr( ImporterClass_->create< CsResourceImporter >() );
}

BcU32 CsResourceImporterAttribute::getVersionId() const
{
	return VersionId_;
}

BcU32 CsResourceImporterAttribute::getPriority() const
{
	return Priority_;
}

//////////////////////////////////////////////////////////////////////////
// Custom deleter.
void CsResourceImporterDeleter::operator() ( class CsResourceImporter* Importer )
{
	if( Importer != nullptr )
	{
		delete Importer;
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsResourceImporter::CsResourceImporter():
	MessageCount_( { 0, 0, 0, 0 } ),
	Importer_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsResourceImporter::CsResourceImporter(
		const std::string& Name,
		const std::string& Type ):
	Name_( Name ),
	Type_( Type ),
	MessageCount_( { 0, 0, 0, 0 } ),
	Importer_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
CsResourceImporter::~CsResourceImporter()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void CsResourceImporter::initialise( 
	class CsPackageImporter* Importer,
	BcU32 ResourceId )
{
	Importer_ = Importer;
	ResourceId_ = ResourceId;
}

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool CsResourceImporter::import( 
	const Json::Value& Object )
{
	BcBreakpoint;
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// getResourceName
std::string CsResourceImporter::getResourceName() const
{
	return Name_;
}

//////////////////////////////////////////////////////////////////////////
// getResourceType
std::string CsResourceImporter::getResourceType() const
{
	return Type_;
}

//////////////////////////////////////////////////////////////////////////
// getResourceId
BcU32 CsResourceImporter::getResourceId() const
{
	return ResourceId_;
}

//////////////////////////////////////////////////////////////////////////
// getImporterAttribute
const CsResourceImporterAttribute* CsResourceImporter::getImporterAttribute() const
{
	auto Attr = static_cast< CsResourceImporterAttribute* >( 
		ReManager::GetClass( getResourceType() )->getAttribute( CsResourceImporterAttribute::StaticGetClass() ) );
	BcAssert( Attr );
	return Attr;
}

//////////////////////////////////////////////////////////////////////////
// addMessage
void CsResourceImporter::addMessage( CsMessageCategory Category, const std::string& Message )
{
	static std::array< const char*, (size_t)CsMessageCategory::MAX > Categories =
	{
		"INFO",
		"WARNING",
		"ERROR",
		"CRITICAL"
	};

	size_t CategoryIdx = (size_t)Category;
	const char* CategoryMsg = Categories[ CategoryIdx ];

	// Increment number of types of messages.
	MessageCount_[ CategoryIdx ]++;

	PSY_LOG( "%s: %s\n", CategoryMsg, Message.c_str() );
}

//////////////////////////////////////////////////////////////////////////
// getMessageCount
size_t CsResourceImporter::getMessageCount( CsMessageCategory Category ) const
{
	size_t CategoryIdx = (size_t)Category;

	return MessageCount_[ CategoryIdx ];
}
	
//////////////////////////////////////////////////////////////////////////
// addImport_DEPRECATED
BcU32 CsResourceImporter::addImport_DEPRECATED( 
	const Json::Value& Resource, 
	BcBool IsCrossRef )
{
#if PSY_IMPORT_PIPELINE
	BcAssert( Importer_ != nullptr );
	return Importer_->addImport( Resource, IsCrossRef );
#else
	return BcErrorCode;
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// addImport
BcU32 CsResourceImporter::addImport( 
	CsResourceImporterUPtr Importer, 
	BcBool IsCrossRef )
{
#if PSY_IMPORT_PIPELINE
	return Importer_->addImport( std::move( Importer ), Json::nullValue, IsCrossRef );
#else
	return BcErrorCode;
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// addString
BcU32 CsResourceImporter::addString( 
	const BcChar* pString )
{
#if PSY_IMPORT_PIPELINE
	BcAssert( Importer_ != nullptr );
	return Importer_->addString( pString );
#else
	return BcErrorCode;
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// addPackageCrossRef
BcU32 CsResourceImporter::addPackageCrossRef( 
	const BcChar* pFullName )
{
#if PSY_IMPORT_PIPELINE
	BcAssert( Importer_ != nullptr );
	return Importer_->addPackageCrossRef( pFullName );
#else
	return BcErrorCode;
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// addChunk
BcU32 CsResourceImporter::addChunk( 
	BcU32 ID, 
	const void* pData, 
	BcSize Size, 
	BcSize RequiredAlignment, 
	BcU32 Flags )
{
#if PSY_IMPORT_PIPELINE
	BcAssert( Importer_ != nullptr );

	// Add to importer.
	return Importer_->addChunk(
		ID,
		pData,
		Size,
		RequiredAlignment,
		Flags );
#else
	return BcErrorCode;
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// addFile
CsFileHash CsResourceImporter::addFile(
	std::string FileName )
{
#if PSY_IMPORT_PIPELINE
	BcFile InFile;
	CsFileHash FileHash;
	if( InFile.open( FileName.c_str() ) )
	{
		auto Bytes = InFile.readAllBytes();
		boost::uuids::detail::sha1 Hasher;
		Hasher.process_block( Bytes, Bytes + InFile.size() );
		Hasher.get_digest( FileHash.Hash_ );

		auto OutFileName = 
			*CsCore::pImpl()->getPackagePackedPath( BcName::INVALID ) + 
			std::string( "/" ) + 
			FileHash.getName() + std::string( ".dat" );

		BcFile OutFile;
		if( OutFile.open( OutFileName.c_str(),  bcFM_WRITE ) )
		{
			OutFile.write( Bytes, InFile.size() );
			OutFile.close();
		}

		BcMemFree( Bytes );
		InFile.close();
	}
	else
	{
		throw new CsImportException( "Unable to open file", FileName );
	}

	return FileHash;
#else
	return CsFileHash();
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// addDependency
void CsResourceImporter::addDependency( 
	const BcChar* pFileName )
{
#if PSY_IMPORT_PIPELINE
	BcAssert( Importer_ != nullptr );
	return Importer_->addDependency( pFileName );
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// addDependency
std::string CsResourceImporter::getIntermediatePath()
{
#if PSY_IMPORT_PIPELINE
	return *CsCore::pImpl()->getIntermediatePath( *Importer_->getName() );
#else
	return "";
#endif // PSY_IMPORT_PIPELINE
}
