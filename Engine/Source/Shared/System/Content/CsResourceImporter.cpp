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

#include <boost/uuid/sha1.hpp>

#include <json/json.h>

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
// Reflection
REFLECTION_DEFINE_BASE( CsResourceImporter );

void CsResourceImporter::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Name_", &CsResourceImporter::Name_, bcRFF_IMPORTER ),
		new ReField( "Type_", &CsResourceImporter::Type_, bcRFF_IMPORTER ),
		new ReField( "Importer_", &CsResourceImporter::Importer_, bcRFF_TRANSIENT ),
	};
	
	ReRegisterClass< CsResourceImporter >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsResourceImporter::CsResourceImporter():
	Importer_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsResourceImporter::CsResourceImporter(
		const std::string& Name,
		const std::string& Type ):
	Name_( Name ),
	Type_( Type )
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
// addImport_DEPRECATED
BcU32 CsResourceImporter::addImport_DEPRECATED( 
	const Json::Value& Resource, 
	BcBool IsCrossRef )
{
	BcAssert( Importer_ != nullptr );
	return Importer_->addImport( Resource, IsCrossRef );
}

//////////////////////////////////////////////////////////////////////////
// addImport
BcU32 CsResourceImporter::addImport( 
	CsResourceImporterUPtr Importer, 
	BcBool IsCrossRef )
{
	return Importer_->addImport( std::move( Importer ), Json::nullValue, IsCrossRef );
}

//////////////////////////////////////////////////////////////////////////
// addString
BcU32 CsResourceImporter::addString( 
	const BcChar* pString )
{
	BcAssert( Importer_ != nullptr );
	return Importer_->addString( pString );
}

//////////////////////////////////////////////////////////////////////////
// addPackageCrossRef
BcU32 CsResourceImporter::addPackageCrossRef( 
	const BcChar* pFullName )
{
	BcAssert( Importer_ != nullptr );
	return Importer_->addPackageCrossRef( pFullName );
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
	BcAssert( Importer_ != nullptr );

	// Add to importer.
	return Importer_->addChunk(
		ID,
		pData,
		Size,
		RequiredAlignment,
		Flags );
}

//////////////////////////////////////////////////////////////////////////
// addFile
CsFileHash CsResourceImporter::addFile(
	std::string FileName )
{
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
}

//////////////////////////////////////////////////////////////////////////
// addDependency
void CsResourceImporter::addDependency( 
	const BcChar* pFileName )
{
	BcAssert( Importer_ != nullptr );
	return Importer_->addDependency( pFileName );
}

//////////////////////////////////////////////////////////////////////////
// addDependency
std::string CsResourceImporter::getIntermediatePath()
{
	return *CsCore::pImpl()->getIntermediatePath( *Importer_->getName() );
}
