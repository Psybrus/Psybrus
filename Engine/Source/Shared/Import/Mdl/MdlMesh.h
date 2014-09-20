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

#include "Base/BcDebug.h"

#include <map>

//////////////////////////////////////////////////////////////////////////
// MdlMesh
class MdlMesh
{
public:
	MdlMesh();
	~MdlMesh();

	//
	void addIndex( const MdlIndex& Index );
	size_t addVertex( const MdlVertex& Vertex );
	size_t addMaterial( const MdlMaterial& Material );
	void bonePalette( const MdlBonePalette& BonePalette );

	// Accessors
	const MdlIndex& index( size_t iIndex ) const;
	const MdlVertex& vertex( size_t iVertex ) const;
	const MdlMaterial& material( size_t iMaterial ) const;
	const MdlBonePalette& bonePalette() const;

	void index( size_t iIndex, const MdlIndex& Index );
	void vertex( size_t iVertex, const MdlVertex& Vertex );
	void material( size_t iMaterial, const MdlMaterial& Material );

	size_t nIndices() const;
	size_t nVertices() const;
	size_t nMaterials() const;

	// Add a vertex that can be shared.
	size_t addVertexShared( const MdlVertex& Vertex );

	// Compare 2 vertices numerically.
	BcBool compareVertices( const MdlVertex& VertexA, const MdlVertex& VertexB );

	// Sort indices by their material to make export easier.
	void sortIndicesByMaterial();

	// Will return first index, and number of indices for a material.
	BcBool materialIndexCount( size_t iMaterial, size_t& iFirst, size_t& nIndices );

	// Transform.
	void bakeTransform( const MaMat4d& Transform );

	//
	void buildNormals();
	void buildTangents();

	//
	void flipIndices();

	//
	MaAABB findAABB() const;

	size_t findBoneCount() const;

	// Split up mesh by material.
	std::vector< MdlMesh >& splitByMaterial();

	// Split up mesh into bone palettes.
	std::vector< MdlMesh >& splitIntoBonePalettes( BcU32 PaletteSize );

	// Flip on z to interchange left and right coordinate systems.
	void flipCoordinateSpace();

	//void optimiseIndices();
	//void generateNormals();
	//void generateTangents();

private:
	MdlIndexArray		aIndices_;
	MdlVertexArray		aVertices_;
	MdlMaterialArray	aMaterials_;

	std::map< BcU32, size_t > aVertexHashes_;

	MdlBonePalette		BonePalette_;
	std::vector< MdlMesh > SubMeshes_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

inline const MdlIndex& MdlMesh::index( size_t iIndex ) const
{
	BcAssert( iIndex < aIndices_.size() );

	return aIndices_[ iIndex ];
}

inline const MdlVertex& MdlMesh::vertex( size_t iVertex ) const
{
	BcAssert( iVertex < aVertices_.size() );

	return aVertices_[ iVertex ];
}

inline const MdlMaterial& MdlMesh::material( size_t iMaterial ) const
{
	BcAssert( iMaterial < aMaterials_.size() );

	return aMaterials_[ iMaterial ];
}

inline const MdlBonePalette& MdlMesh::bonePalette() const
{
	return BonePalette_;
}


inline void MdlMesh::index( size_t iIndex, const MdlIndex& Index )
{
	BcAssert( iIndex < aIndices_.size() );

	aIndices_[ iIndex ] = Index;
}

inline void MdlMesh::vertex( size_t iVertex, const MdlVertex& Vertex )
{
	BcAssert( iVertex < aVertices_.size() );

	aVertices_[ iVertex ] = Vertex;
}

inline void MdlMesh::material( size_t iMaterial, const MdlMaterial& Material )
{
	BcAssert( iMaterial < aMaterials_.size() );

	aMaterials_[ iMaterial ] = Material;
}

inline void MdlMesh::bonePalette( const MdlBonePalette& BonePalette )
{
	BonePalette_ = BonePalette;
}

inline size_t MdlMesh::nIndices() const
{
	return aIndices_.size();
}

inline size_t MdlMesh::nVertices() const
{
	return aVertices_.size();
}

inline size_t MdlMesh::nMaterials() const
{
	return aMaterials_.size();
}

#endif
