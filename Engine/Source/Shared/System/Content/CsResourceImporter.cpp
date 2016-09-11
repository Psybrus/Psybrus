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
#include <DR_SHA1.h> // from RakNet.
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
REFLECTION_DEFINE_DERIVED( CsResourceImporter );

void CsResourceImporter::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Type_", &CsResourceImporter::Type_, bcRFF_IMPORTER ),
		new ReField( "MessageCount_", &CsResourceImporter::MessageCount_ ),
		new ReField( "Importer_", &CsResourceImporter::Importer_, bcRFF_TRANSIENT ),
	};
	
	ReRegisterAbstractClass< CsResourceImporter, Super >( Fields );
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
	Importer_( nullptr ),
	ResourceId_( BcErrorCode )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsResourceImporter::CsResourceImporter(
		const std::string& Name,
		const std::string& Type ):
	Type_( Type ),
	MessageCount_( { 0, 0, 0, 0 } ),
	Importer_( nullptr ),
	ResourceId_( BcErrorCode )
{
	setName( Name );
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
// getPackageName
std::string CsResourceImporter::getPackageName() const
{
#if PSY_IMPORT_PIPELINE
	return *Importer_->getName();
#else
	return "";
#endif
}

//////////////////////////////////////////////////////////////////////////
// getResourceName
std::string CsResourceImporter::getResourceName() const
{
	return *getName();
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
	BcAssertMsg( Attr, "Class \"%s\" has no importer attribute.", getResourceType().c_str() );
	return Attr;
}

//////////////////////////////////////////////////////////////////////////
// addMessage
void CsResourceImporter::addMessage( CsMessageCategory Category, const char* Message, ... )
{
	std::array< char, 1024 > FormattedMessage;
	va_list Args;
	va_start( Args, Message );
	BcVSPrintf( FormattedMessage.data(), FormattedMessage.size(), Message, Args ); 
	va_end( Args );


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

	PSY_LOG( "%s: %s\n", CategoryMsg, FormattedMessage.data() );
}

//////////////////////////////////////////////////////////////////////////
// getMessageCount
size_t CsResourceImporter::getMessageCount( CsMessageCategory Category ) const
{
	size_t CategoryIdx = (size_t)Category;

	return MessageCount_[ CategoryIdx ];
}

//////////////////////////////////////////////////////////////////////////
// addImport
const ReObject* CsResourceImporter::getImportParams( const ReClass* Class ) const
{
#if PSY_IMPORT_PIPELINE
	return Importer_->getParams().getImportParams( Class );
#else
	return nullptr;
#endif
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
		CSHA1 Hasher;
		Hasher.Update( Bytes.get(), InFile.size() );
		Hasher.Final();
		if( Hasher.GetHash( reinterpret_cast< unsigned char* >( &FileHash.Hash_[ 0 ] ) ) )
		{
			auto PackedPath = Importer_->getParams().getPackagePackedPath( BcName::INVALID );
			auto OutFileName = 
				*PackedPath + 
				std::string( "/" ) + 
				FileHash.getName() + std::string( ".dat" );

			BcFile OutFile;
			if( OutFile.open( OutFileName.c_str(),  bcFM_WRITE ) )
			{
				OutFile.write( Bytes.get(), InFile.size() );
				OutFile.close();
			}

			InFile.close();
		}
		else
		{
			throw new CsImportException( "Unable to get hash", FileName.c_str() );
		}
	}
	else
	{
		throw new CsImportException( "Unable to open file", FileName.c_str() );
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
void CsResourceImporter::addDependency( 
	const ReClass* Class )
{
#if PSY_IMPORT_PIPELINE
	BcAssert( Importer_ != nullptr );
	return Importer_->addDependency( Class );
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// addDependency
std::string CsResourceImporter::getIntermediatePath()
{
#if PSY_IMPORT_PIPELINE
	return *Importer_->getParams().getPackageIntermediatePath( *Importer_->getName() );
#else
	return "";
#endif // PSY_IMPORT_PIPELINE
}
