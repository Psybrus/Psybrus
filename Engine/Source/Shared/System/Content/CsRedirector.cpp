/**************************************************************************
*
* File:		CsRedirector.cpp
* Author:	Neil Richardson
* Ver/Date:	
* Description:
*			Resource used to redirect to another. Will all be transparent 
*			to the user of the content system at the CsCore & CsPackage 
*			level.
*
*
**************************************************************************/

#include "System/Content/CsRedirector.h"
#include "System/Content/CsPackage.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( CsRedirectorImport )
	
void CsRedirectorImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Target_", &CsRedirectorImport::Target_, bcRFF_IMPORTER ),
	};
		
	ReRegisterClass< CsRedirectorImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsRedirectorImport::CsRedirectorImport():
	Target_( CSCROSSREFID_INVALID )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsRedirectorImport::CsRedirectorImport( ReNoInit ):
	Target_( CSCROSSREFID_INVALID )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsRedirectorImport::~CsRedirectorImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool CsRedirectorImport::import( const Json::Value& )
{
	BcBool RetVal = BcFalse;
#ifdef PSY_IMPORT_PIPELINE
	if( Target_ != CSCROSSREFID_INVALID )
	{
		CsResourceImporter::addChunk( BcHash( "target" ), &Target_, sizeof( Target_ ), sizeof( Target_ ), csPCF_IN_PLACE );
		RetVal = BcTrue;
	}
#endif
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( CsRedirector );

void CsRedirector::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "CrossRefId_", &CsRedirector::CrossRefId_ ),
		new ReField( "Target_", &CsRedirector::Target_ ),
	};
		
	auto& Class = ReRegisterClass< CsRedirector, Super >( Fields );
	BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		CsRedirectorImport::StaticGetClass(), 0 ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void CsRedirector::initialise()
{
	CrossRefId_ = CSCROSSREFID_INVALID;
	Target_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getResource
CsResource* CsRedirector::getResource()
{
	return Target_;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void CsRedirector::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0, &CrossRefId_ );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void CsRedirector::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "target" ) )
	{
		// Get target resource.
		Target_ = getPackage()->getCrossRefResource( CrossRefId_ );

		// Mark as ready to use.
		markCreate();
		markReady();
	}
}

