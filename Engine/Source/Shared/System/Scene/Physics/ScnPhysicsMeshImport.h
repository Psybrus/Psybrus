/**************************************************************************
*
* File:		ScnPhysicsMeshImport.h
* Author:	Neil Richardson 
* Ver/Date: 
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNPHYSICSMESHIMPORT_H__
#define __SCNPHYSICSMESHIMPORT_H__

#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"

#include "System/Scene/Physics/ScnPhysicsFileData.h"

#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsMeshImport
class ScnPhysicsMeshImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsMeshImport, CsResourceImporter );

public:
	ScnPhysicsMeshImport();
	virtual ~ScnPhysicsMeshImport();

	/**
	 * Import.
	 */
	BcBool import( const Json::Value& );

private:
	ScnPhysicsMeshShapeType ShapeType_;
	std::string Source_;
	std::map< std::string, CsCrossRefId > Materials_;


	BcStream HeaderStream_;
	BcStream TriangleStream_;
	BcStream VertexStream_;

	const struct aiScene* Scene_;

};

#endif
