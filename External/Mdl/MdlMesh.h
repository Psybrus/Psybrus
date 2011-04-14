/**************************************************************************
*
* File:		MdlMesh.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Static mesh container.
*		
*
*
* 
**************************************************************************/

#ifndef __MDLMESH_H__
#define __MDLMESH_H__

#include "MdlTypes.h"

#include "BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// MdlMesh
class MdlMesh
{
public:
	MdlMesh();
	~MdlMesh();

	//
	void addIndex( const MdlIndex& Index );
	BcU32 addVertex( const MdlVertex& Vertex );
	BcU32 addMaterial( const MdlMaterial& Material );

	// Accessors
	const MdlIndex& index( BcU32 iIndex ) const;
	const MdlVertex& vertex( BcU32 iVertex ) const;
	const MdlMaterial& material( BcU32 iMaterial ) const;

	void index( BcU32 iIndex, const MdlIndex& Index );
	void vertex( BcU32 iVertex, const MdlVertex& Vertex );
	void material( BcU32 iMaterial, const MdlMaterial& Material );

	BcU32 nIndices() const;
	BcU32 nVertices() const;
	BcU32 nMaterials() const;

	// Add a vertex that can be shared.
	BcU32 addVertexShared( const MdlVertex& Vertex );

	// Compare 2 vertices numerically.
	BcBool compareVertices( const MdlVertex& VertexA, const MdlVertex& VertexB );

	// Sort indices by their material to make export easier.
	void sortIndicesByMaterial();

	// Will return first index, and number of indices for a material.
	BcBool materialIndexCount( BcU32 iMaterial, BcU32& iFirst, BcU32& nIndices );

	// Count the number of joints used by a material/group
	BcU32 countJointsUsed( BcU32 iMaterial );

	// Transform.
	void bakeTransform( const BcMat4d& Transform );

	//
	void buildNormals();
	void buildTangents();

	//
	void flipIndices();

	//
	BcAABB findAABB() const;

	// Split up mesh by material.
	const std::vector< MdlMesh >& splitByMaterial();

	// Flip on z to interchange left and right coordinate systems.
	void flipCoordinateSpace();

	//void optimiseIndices();
	//void generateNormals();
	//void generateTangents();

private:
	MdlIndexArray		aIndices_;
	MdlVertexArray		aVertices_;
	MdlMaterialArray	aMaterials_;

	std::vector< MdlMesh > SubMeshes_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

inline const MdlIndex& MdlMesh::index( BcU32 iIndex ) const
{
	BcAssert( iIndex < aIndices_.size() );

	return aIndices_[ iIndex ];
}

inline const MdlVertex& MdlMesh::vertex( BcU32 iVertex ) const
{
	BcAssert( iVertex < aVertices_.size() );

	return aVertices_[ iVertex ];
}

inline const MdlMaterial& MdlMesh::material( BcU32 iMaterial ) const
{
	BcAssert( iMaterial < aMaterials_.size() );

	return aMaterials_[ iMaterial ];
}

inline void MdlMesh::index( BcU32 iIndex, const MdlIndex& Index )
{
	BcAssert( iIndex < aIndices_.size() );

	aIndices_[ iIndex ] = Index;
}

inline void MdlMesh::vertex( BcU32 iVertex, const MdlVertex& Vertex )
{
	BcAssert( iVertex < aVertices_.size() );

	aVertices_[ iVertex ] = Vertex;
}

inline void MdlMesh::material( BcU32 iMaterial, const MdlMaterial& Material )
{
	BcAssert( iMaterial < aMaterials_.size() );

	aMaterials_[ iMaterial ] = Material;
}


inline BcU32 MdlMesh::nIndices() const
{
	return (BcU32)aIndices_.size();
}

inline BcU32 MdlMesh::nVertices() const
{
	return (BcU32)aVertices_.size();
}

inline BcU32 MdlMesh::nMaterials() const
{
	return (BcU32)aMaterials_.size();
}

#endif
