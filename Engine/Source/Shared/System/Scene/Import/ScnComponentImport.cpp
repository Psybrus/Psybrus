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

#if PSY_IMPORT_PIPELINE

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
ScnComponentImport::ScnComponentImport()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnComponentImport::ScnComponentImport( ReNoInit )
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
BcBool ScnComponentImport::import(
		const Json::Value& Object )
{
	// Check we have a valid type to serialise in.
	if( Object[ "type" ].type() != Json::stringValue )
	{
		return BcFalse;
	}
	class ReClass* ComponentClass = ReManager::GetClass( Object[ "type" ].asCString() );
	if( ComponentClass == nullptr )
	{
		return BcFalse;
	}

	// Need to finish this code path off.
	// - Need to ensure the resource created is destroyed,
	//   otherwise it will crash in CsCore.
	// - Need to save as string, or better yet - binary.
#if 0
	// Create the component + initialise it.
	auto Component = 
		std::unique_ptr< ScnComponent >( ComponentClass->create< ScnComponent >() );
	Component->initialise();

	// Serialise object onto component.
	// TODO: Pointer types. We should be able to spawn them,
	//       but we only want their names cached.
	CsSerialiserPackageObjectCodec ObjectCodec( nullptr, bcRFF_ALL, bcRFF_TRANSIENT );
	SeJsonReader Reader( &ObjectCodec );
	Reader.serialiseClassMembers( Component.get(), Component->getClass(), Object );
	
	SeJsonWriter Writer( &ObjectCodec );
	Writer.serialise( Component.get(), Component->getClass() );
	Writer.save( "test_component.json" );
#endif

	// Write out object to be used later.
	Json::FastWriter JsonWriter;
	std::string JsonData = JsonWriter.write( Object );
	
	//
	CsResourceImporter::addChunk( BcHash( "object" ), JsonData.c_str(), JsonData.size() + 1 );
	return BcTrue;
}

#endif // PSY_IMPORT_PIPELINE
