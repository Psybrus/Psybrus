/**************************************************************************
*
* File:		ScnEntityImport.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Import/ScnEntityImport.h"

#include "System/Scene/ScnEntity.h"

#include "Base/BcFile.h"
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnEntityImport )
	
void ScnEntityImport::StaticRegisterClass()
{
	/*
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnEntityImport::Source_ ),
	};
	*/
	
	ReRegisterClass< ScnEntityImport, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnEntityImport::ScnEntityImport()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnEntityImport::ScnEntityImport( ReNoInit )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnEntityImport::~ScnEntityImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnEntityImport::import(
		const Json::Value& Object )
{
#if PSY_IMPORT_PIPELINE
	//
	Json::Value Components = Object[ "components" ];

	BcStream Stream;
	ScnEntityHeader Header;
	Header.NoofComponents_ = Components.size();
	Stream << Header;
	for( BcU32 Idx = 0; Idx < Components.size(); ++Idx )
	{
		Json::Value& Component( Components[ Idx ] );
		auto Class = Component[ "$Class" ];
		if( Class == Json::nullValue )
		{
			Class = Component[ "type" ];
		}
		
		// Create a vaguely unique name.
		if( Component.get( "name", Json::nullValue ).type() == Json::nullValue )
		{
			Component[ "name" ] = (*BcName( Class.asCString() ).getUnique());
		}

		BcU32 CrossRef = CsResourceImporter::addImport_DEPRECATED( Component, BcTrue );
		Stream << CrossRef;
	}	

	CsResourceImporter::addChunk( BcHash( "header" ), Stream.pData(), Stream.dataSize() );

	return BcTrue;
#else
	return BcFalse;
#endif // PSY_IMPORT_PIPELINE
}

