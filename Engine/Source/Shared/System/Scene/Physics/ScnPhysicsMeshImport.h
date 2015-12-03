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
	BcBool import( const Json::Value& ) override;

private:
	ScnPhysicsMeshShapeType ShapeType_;
	std::string Source_;
	std::map< std::string, CsCrossRefId > Materials_;

	// HACD Parameters.
	// Documentation comes from hacdHACD.h.

	/// Sets the minimum number of clusters to be generated.
	BcU32 MinClusters_;
	/// Sets the maximum number of vertices per cluster.
	BcU32 MaxVerticesPerCluster_;
	/// Sets the maximum allowed concavity.
	BcF32 Concavity_;
	/// Specifies whether extra points should be added when computing the concavity.
	BcBool AddExtraDistPoints_;
	/// Specifies whether extra points should be added when computing the concavity.
	BcBool AddNeighbourDistPoints_;
	/// Specifies whether faces points should be added when computing the concavity.
	BcBool AddFacePoints_;


	BcStream HeaderStream_;
	BcStream MeshPartStream_;
	BcStream TriangleStream_;
	BcStream VertexStream_;

	const struct aiScene* Scene_;

};

#endif
