/**************************************************************************
*
* File:		ScnModelImport.h
* Author:	Neil Richardson 
* Ver/Date: 06/01/13
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNMODELIMPORT_H__
#define __SCNMODELIMPORT_H__

#ifdef PSY_SERVER
#include "System/Content/CsCore.h"
#include "System/Scene/ScnModelFileData.h"

#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// ScnModelImport
class ScnModelImport
{
public:
	ScnModelImport();

	/**
	 * Import.
	 */
	BcBool import( 
		class CsPackageImporter& Importer, 
		const Json::Value& Object );

private:
	// old Mdl library code.
	void recursiveSerialiseNodes( 
		class MdlNode* pNode,
		BcU32 ParentIndex,
		BcU32& NodeIndex,
		BcU32& PrimitiveIndex );

	void serialiseMesh( 
		class MdlMesh* pSkin,
		BcU32 ParentIndex,
		BcU32& NodeIndex,
		BcU32& PrimitiveIndex );

	void serialiseSkin( 
		class MdlMesh* pSkin,
		BcU32 ParentIndex,
		BcU32& NodeIndex,
		BcU32& PrimitiveIndex );

	void serialiseVertices( 
		class MdlMesh* pMesh,
		RsVertexElement* pVertexElements,
		BcU32 NoofVertexElements,
		MaAABB& AABB );


	// new assimp code.
	void recursiveSerialiseNodes( 
		struct aiNode* pNode,
		BcU32 ParentIndex,
		BcU32& NodeIndex,
		BcU32& PrimitiveIndex );

	void serialiseMesh( 
		struct aiMesh* Mesh,
		BcU32 ParentIndex,
		BcU32& NodeIndex,
		BcU32& PrimitiveIndex );

	void serialiseVertices( 
		struct aiMesh* Mesh,
		RsVertexElement* pVertexElements,
		BcU32 NoofVertexElements,
		MaAABB& AABB );

private:
	std::string Source_;

	CsPackageImporter* pImporter_;
	BcStream HeaderStream_;
	BcStream NodeTransformDataStream_;
	BcStream NodePropertyDataStream_;
	BcStream VertexDataStream_;
	BcStream IndexDataStream_;
	BcStream VertexElementStream_;
	BcStream MeshDataStream_;

	const struct aiScene* Scene_;
};

#endif

#endif
