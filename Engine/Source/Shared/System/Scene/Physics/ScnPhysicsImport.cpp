/**************************************************************************
*
* File:		ScnPhysicsImport.cpp
* Author:	Neil Richardson 
* Ver/Date: 25/02/13
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnPhysicsImport.h"

#if PSY_IMPORT_PIPELINE

#include "ScnPhysicsFileData.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsImport::ScnPhysicsImport()
{
	
}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnPhysicsImport::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	// TODO: Clean out into individual classes...hack for easy for now.
	pImporter_ = &Importer;

	BcStream HeaderStream;

	if( BcStrCompare( Object[ "type" ].asCString(), "ScnPhysicsBoxCollisionShape" ) )
	{
		ScnPhysicsBoxCollisionShapeHeader Header;
	
		Header.HalfExtents_ = MaVec3d( Object[ "extents" ].asCString() );

		HeaderStream << Header;
	}
	else if( BcStrCompare( Object[ "type" ].asCString(), "ScnPhysicsSphereCollisionShape" ) )
	{
		ScnPhysicsSphereCollisionShapeHeader Header;
	
		Header.Radius_ = static_cast< BcF32 >( Object[ "radius" ].asDouble() );

		HeaderStream << Header;
	}
	else
	{
		return BcFalse;
	}

	Importer.addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );

	return BcTrue;
}

#endif // PSY_IMPORT_PIPELINE
