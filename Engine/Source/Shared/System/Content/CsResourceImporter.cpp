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

#include <json/json.h>

//////////////////////////////////////////////////////////////////////////
// Custom deleter.
void CsResourceImporterDeleter::operator() ( class CsResourceImporter* Importer )
{
	if( Importer != nullptr )
	{
		Importer->getClass()->destruct( Importer );
		Importer->getClass()->freeNoDestruct( Importer );
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

CsResourceImporterAttribute::CsResourceImporterAttribute( 
	const ReClass* ImporterClass ):
	ImporterClass_( ImporterClass )
{

}

CsResourceImporterUPtr CsResourceImporterAttribute::getImporter() const
{
	return CsResourceImporterUPtr( ImporterClass_->construct< CsResourceImporter >() );
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
	class CsPackageImporter* Importer )
{
	Importer_ = Importer;
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
	return Importer_->addChunk(
		ID,
		pData,
		Size,
		RequiredAlignment,
		Flags );
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
	return *CsCore::pImpl()->getPackageIntermediatePath( Importer_->getName() );
}
