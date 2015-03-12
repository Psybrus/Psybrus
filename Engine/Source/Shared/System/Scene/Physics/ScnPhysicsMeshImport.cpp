/**************************************************************************
*
* File:		ScnPhysicsMeshImport.cpp
* Author:	Neil Richardson 
* Ver/Date: 
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsMeshImport.h"

#if PSY_IMPORT_PIPELINE

#include "System/Scene/Physics/ScnPhysicsFileData.h"

#endif // PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnPhysicsMeshImport )
	
void ScnPhysicsMeshImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnPhysicsMeshImport::Source_, bcRFF_IMPORTER ),
	};
	
	ReRegisterClass< ScnPhysicsMeshImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsMeshImport::ScnPhysicsMeshImport()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
//virtual
ScnPhysicsMeshImport::~ScnPhysicsMeshImport()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcBool ScnPhysicsMeshImport::import( const Json::Value& )
{
#if PSY_IMPORT_PIPELINE
#endif // PSY_IMPORT_PIPELINE
	return BcFalse;

}
