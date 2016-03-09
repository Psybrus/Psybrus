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

#include "System/Content/CsSerialiserPackageObjectCodec.h"

#include "Serialisation/SeJsonWriter.h"

#include "Base/BcFile.h"
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnEntityImport )
	
void ScnEntityImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "LocalTransform_", &ScnEntityImport::LocalTransform_, bcRFF_IMPORTER ),
	};
	
	ReRegisterClass< ScnEntityImport, Super >( Fields );
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
	Header.LocalTransform_ = LocalTransform_;
	Header.NoofComponents_ = Components.size();
	Stream << Header;
	for( BcU32 Idx = 0; Idx < Components.size(); ++Idx )
	{
		Json::Value& ComponentObj( Components[ Idx ] );

		// ref hack.
		auto Reference = ComponentObj.get( "$Reference", Json::nullValue );
		if( Reference.type() == Json::nullValue )
		{
			auto Class = ComponentObj[ "$Class" ];
			if( Class == Json::nullValue )
			{
				PSY_LOG( "$Class not defined in component for entity %s.%s. Falling back to type. Deprecated behaviour.",
					getPackageName().c_str(),
					getResourceName().c_str() );
				Class = ComponentObj[ "type" ];
			}

			// Add dependency.
			CsResourceImporter::addDependency( ReManager::GetClass( Class.asCString() ) );
			
			// Create a vaguely unique name.
			if( ComponentObj.get( "name", Json::nullValue ).type() == Json::nullValue )
			{
				ComponentObj[ "name" ] = (*BcName( Class.asCString() ).getUnique());
			}

			BcU32 CrossRef = CsResourceImporter::addImport_DEPRECATED( ComponentObj, BcTrue );
			Stream << CrossRef;
		}
		else
		{
			BcU32 CrossRef = Reference.asUInt();
			Stream << CrossRef;
		}
	}

	CsResourceImporter::addChunk( BcHash( "header" ), Stream.pData(), Stream.dataSize() );

	return BcTrue;
#else
	return BcFalse;
#endif // PSY_IMPORT_PIPELINE
}

