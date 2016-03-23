/**************************************************************************
*
* File:		ScnComponentImport.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Import/ScnComponentImport.h"

#include "Serialisation/SeJsonReader.h"
#include "Serialisation/SeJsonWriter.h"
#include "System/Scene/ScnComponent.h"
#include "System/Content/CsSerialiserPackageObjectCodec.h"

#include "Base/BcFile.h"
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnComponentImport )
	
void ScnComponentImport::StaticRegisterClass()
{
	/*
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnComponentImport::Source_ ),
	};
	*/
		
	ReRegisterClass< ScnComponentImport, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnComponentImport::ScnComponentImport():
	Component_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnComponentImport::ScnComponentImport( ScnComponent* Component ):
	CsResourceImporter( *Component->getName(), *Component->getTypeName() ),
	Component_( Component )
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnComponentImport::ScnComponentImport( ReNoInit ):
	Component_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnComponentImport::~ScnComponentImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnComponentImport::import()
{
#if PSY_IMPORT_PIPELINE
	BcAssertMsg( Component_, "ScnComponentImport needs to be constructed with a component." );
	
	// Write Json out.
	CsSerialiserPackageObjectCodec ObjectCodec( nullptr, bcRFF_IMPORTER, bcRFF_NONE, bcRFF_IMPORTER );
	SeJsonWriter Writer( &ObjectCodec );
	Writer.serialise( Component_, Component_->getClass() );
	Writer.save( "test_component.json" );

	// Write out object to be used later.
	Json::FastWriter JsonWriter;
	std::string JsonData = JsonWriter.write( Writer.getValue() );
	
	//
	CsResourceImporter::addChunk( BcHash( "object" ), JsonData.c_str(), JsonData.size() + 1 );

	// Destroy all cross refs.
	std::set< CsResource* > CrossRefs;
	ReVisitRecursively( Component_, Component_->getClass(),
		[ &CrossRefs ]( void* InData, const ReClass* InClass )
		{
			if( InClass->hasBaseClass( CsResource::StaticGetClass() ) )
			{
				CsResource* Resource = static_cast< CsResource* >( InData );
				if( Resource->getName().getValue() == "$CrossRef" )
				{
					CrossRefs.insert( Resource );
				}
			}
		} );
	for( auto Resource : CrossRefs )
	{
		CsCore::pImpl()->internalForceDestroy( Resource );
	}
	CsCore::pImpl()->internalForceDestroy( Component_ );

	return BcTrue;
#else
	return BcFalse;
#endif // PSY_IMPORT_PIPELINE
}

