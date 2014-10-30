/**************************************************************************
*
* File:		MdlMesh.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Static mesh container.
*		
*
*
* 
**************************************************************************/

#include "MdlMesh.h"

#include "Base/BcHash.h"
#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
MdlMesh::MdlMesh()
{
	// Default bone palette.
	for( BcU32 Idx = 0; Idx < BonePalette_.BonePalette_.size(); ++Idx )
	{
		BonePalette_.BonePalette_[ Idx ] = Idx;
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
MdlMesh::~MdlMesh()
{

}

//////////////////////////////////////////////////////////////////////////
// addIndex
void MdlMesh::addIndex( const MdlIndex& Index )
{
	aIndices_.push_back( Index );
}

//////////////////////////////////////////////////////////////////////////
// addVertex
size_t MdlMesh::addVertex( const MdlVertex& Vertex )
{
	aVertices_.push_back( Vertex );

	BcU32 VertexHash = BcHash::GenerateCRC32( 0, &Vertex, sizeof( Vertex ) ) ;
	size_t VertexIdx = aVertices_.size() - 1;
	aVertexHashes_[ VertexHash ] = VertexIdx;

	return VertexIdx;
}

//////////////////////////////////////////////////////////////////////////
// addMaterial
size_t MdlMesh::addMaterial( const MdlMaterial& Material )
{
	BcU32 iMaterial = 0;
	BcBool bFoundMaterial = BcFalse;

	for( BcU32 i = 0; i < aMaterials_.size(); ++i )
	{
		if( aMaterials_[ i ] == Material )
		{
			iMaterial = i;
			bFoundMaterial = BcTrue;
			break;
		}
	}

	if( bFoundMaterial == BcFalse )
	{
		aMaterials_.push_back( Material );
		iMaterial = (BcU32)( aMaterials_.size() - 1 );
	}
	return iMaterial;
}

//////////////////////////////////////////////////////////////////////////
// addVertexShared
size_t MdlMesh::addVertexShared( const MdlVertex& Vertex )
{
	// Go through all our vertices, and if we find a matching one,
	// return the index to it, else just add this vert and return
	// the index to it.
	size_t iVertex = (size_t)-1;

	std::map< BcU32, size_t >::iterator VertexIt = aVertexHashes_.find( BcHash::GenerateCRC32( 0, &Vertex, sizeof( Vertex ) ) );
	if( VertexIt != aVertexHashes_.end() )
	{
		iVertex = VertexIt->second;
		BcAssert( compareVertices( Vertex, aVertices_[ iVertex ] ) );
	}

	// If we didn't find a vertex then add it.
	if( iVertex == -1 )
	{
		iVertex = addVertex( Vertex );
	}

	return iVertex;
}

//////////////////////////////////////////////////////////////////////////
// compareVertices
BcBool MdlMesh::compareVertices( const MdlVertex& VertexA, const MdlVertex& VertexB )
{
	// Could do a hash key comparison, or possibly the whole structure.
	// Just to be safe though...since we can optimise out crappy
	// vertices here too!

	BcBool bRetVal = BcTrue;

	bRetVal &= ( VertexA.Position_ == VertexB.Position_ );
	bRetVal &= ( VertexA.bPosition_ == VertexB.bPosition_ );

	bRetVal &= ( VertexA.Normal_ == VertexB.Normal_ );
	bRetVal &= ( VertexA.bNormal_ == VertexB.bNormal_ );

	bRetVal &= ( VertexA.Tangent_ == VertexB.Tangent_ );
	bRetVal &= ( VertexA.bTangent_ == VertexB.bTangent_ );

	bRetVal &= ( VertexA.UV_ == VertexB.UV_ );
	bRetVal &= ( VertexA.bUV_ == VertexB.bUV_ );

	bRetVal &= ( VertexA.Colour_ == VertexB.Colour_ );
	bRetVal &= ( VertexA.bColour_ == VertexB.bColour_ );

	bRetVal &= ( VertexA.nWeights_ == VertexB.nWeights_ );

	for( BcU32 i = 0; i < VertexA.nWeights_; ++i )
	{
		bRetVal &= VertexA.iJoints_[ i ] == VertexB.iJoints_[ i ];
		bRetVal &= VertexA.Weights_[ i ] == VertexB.Weights_[ i ];
	}

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
// sortIndicesByMaterial
void MdlMesh::sortIndicesByMaterial()
{
	// This could be improved performance wise by using radix or quicksort.
	MdlIndexArray aNewIndices_;

	aNewIndices_.reserve( aIndices_.size() );

	for( BcU32 iMaterial = 0; iMaterial < aMaterials_.size(); ++iMaterial )
	{
		// Add each index thats bound to this material to the new list.
		for( BcU32 iIndex = 0; iIndex < aIndices_.size(); ++iIndex )
		{
			if( aIndices_[ iIndex ].iMaterial_ == iMaterial )
			{
				aNewIndices_.push_back( aIndices_[ iIndex ] );
			}
		}
	}

	// Copy new array over the top one.
	if( aMaterials_.size() > 0 )
	{
		aIndices_ = aNewIndices_;
	}
}


//////////////////////////////////////////////////////////////////////////
// materialIndexCount
BcBool MdlMesh::materialIndexCount( size_t iMaterial, size_t& iFirst, size_t& nIndices )
{
	BcBool bFound = BcFalse;

	// Find first index for material.
	for( BcU32 i = 0; i < aIndices_.size(); ++i )
	{
		if( aIndices_[ i ].iMaterial_ == iMaterial )
		{
			iFirst = i;
			bFound = BcTrue;
			break;
		}
	}

	if( bFound == BcFalse )
	{
		return BcFalse;
	}

	// Now count how many we have.
	nIndices = 0;
	for( size_t i = iFirst; i < aIndices_.size(); ++i )
	{
		if( aIndices_[ i ].iMaterial_ == iMaterial )
		{
			++nIndices;
		}
		else
		{
			break;
		}
	}

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// bakeTransform
void MdlMesh::bakeTransform( const MaMat4d& Transform )
{
	MaMat4d NrmTransform = Transform;
	NrmTransform.translation( MaVec3d( 0.0f, 0.0f, 0.0f ) );
	for( size_t i = 0; i < aVertices_.size(); ++i )
	{
		aVertices_[ i ].Position_ = aVertices_[ i ].Position_ * Transform;
		aVertices_[ i ].Normal_ = aVertices_[ i ].Normal_ * NrmTransform;
		aVertices_[ i ].Normal_.normalise();
		aVertices_[ i ].Tangent_ = aVertices_[ i ].Tangent_ * NrmTransform;
		aVertices_[ i ].Tangent_.normalise();
	}
}

//////////////////////////////////////////////////////////////////////////
// buildNormals
void MdlMesh::buildNormals()
{
	for ( size_t i = 0; i < ( aIndices_.size() / 3 ); ++i )
	{
		size_t TA = aIndices_[ ( i * 3 ) + 0 ].iVertex_;
		size_t TB = aIndices_[ ( i * 3 ) + 1 ].iVertex_;
		size_t TC = aIndices_[ ( i * 3 ) + 2 ].iVertex_;

		MdlVertex& VertA = aVertices_[ TA ];
		MdlVertex& VertB = aVertices_[ TB ];
		MdlVertex& VertC = aVertices_[ TC ];

		MaVec3d VertPosA( VertA.Position_.x(), VertA.Position_.y(), VertA.Position_.z() );
		MaVec3d VertPosB( VertB.Position_.x(), VertB.Position_.y(), VertB.Position_.z() );
		MaVec3d VertPosC( VertC.Position_.x(), VertC.Position_.y(), VertC.Position_.z() );

		MaVec3d Normal = ( VertPosA - VertPosB ).cross( ( VertPosB - VertPosC ) );

		VertA.Normal_ += Normal;
		VertB.Normal_ += Normal;
		VertC.Normal_ += Normal;
	}

	for ( BcU32 i = 0; i < aVertices_.size(); ++i )
	{
		MdlVertex& Vert = aVertices_[ i ];
		Vert.bNormal_ = BcTrue;
		Vert.Normal_.normalise();

		//BcF32 Mag = Vert.Normal_.magnitude(); 
		//BcAssert( BcAbs( Mag - 1.0f ) < 0.00001f );
	}
}

//////////////////////////////////////////////////////////////////////////
// buildTangents
void MdlMesh::buildTangents()
{
	MaVec3d* pTan1 = new MaVec3d[ aVertices_.size() * 2 ];
	MaVec3d* pTan2 = pTan1 + aVertices_.size();

	memset( pTan1, 0, sizeof( MaVec3d ) * aVertices_.size() * 2 );

	for ( size_t i = 0; i < ( aIndices_.size() / 3 ); ++i )
	{
		size_t TA = aIndices_[ ( i * 3 ) + 0 ].iVertex_;
		size_t TB = aIndices_[ ( i * 3 ) + 1 ].iVertex_;
		size_t TC = aIndices_[ ( i * 3 ) + 2 ].iVertex_;

		MdlVertex& VertA = aVertices_[ TA ];
		MdlVertex& VertB = aVertices_[ TB ];
		MdlVertex& VertC = aVertices_[ TC ];

		MaVec3d VertPosA = VertA.Position_;
		MaVec3d VertPosB = VertB.Position_;
		MaVec3d VertPosC = VertC.Position_;

		MaVec2d VertUVA = VertA.UV_;
		MaVec2d VertUVB = VertB.UV_;
		MaVec2d VertUVC = VertC.UV_;

		BcF32 X1 = VertPosB.x() - VertPosA.x();
		BcF32 X2 = VertPosC.x() - VertPosA.x();
		BcF32 Y1 = VertPosB.y() - VertPosA.y();
		BcF32 Y2 = VertPosC.y() - VertPosA.y();
		BcF32 Z1 = VertPosB.z() - VertPosA.z();
		BcF32 Z2 = VertPosC.z() - VertPosA.z();

		BcF32 S1 = VertUVB.x() - VertUVA.x();
		BcF32 S2 = VertUVC.x() - VertUVA.x();
		BcF32 T1 = VertUVB.y() - VertUVA.y();
		BcF32 T2 = VertUVC.y() - VertUVA.y();

		BcF32 InvR = ( S1 * T2 - S2 * T1 );
		BcF32 R = 1.0f / InvR;

		// Validation so it doesn't break everything, just set to a dummy value.
		if( BcAbs( InvR ) < 1e-6f )
		{
			R = 0.0f;
		}

		MaVec3d SDir( ( T2 * X1 - T1 * X2 ) * R, ( T2 * Y1 - T1 * Y2 ) * R, ( T2 * Z1 - T1 * Z2 ) * R );
		MaVec3d TDir( ( S1 * X2 - S2 * X1 ) * R, ( S1 * Y2 - S2 * Y1 ) * R, ( S1 * Z2 - S2 * Z1 ) * R );

		pTan1[ TA ] += SDir;
		pTan1[ TB ] += SDir;
		pTan1[ TC ] += SDir;

		pTan2[ TA ] += TDir;
		pTan2[ TB ] += TDir;
		pTan2[ TC ] += TDir;
	}

	for ( size_t i = 0; i < aVertices_.size(); ++i )
	{
		MdlVertex& Vert = aVertices_[ i ];
		MaVec3d Tangent;

		const MaVec3d N = Vert.Normal_;
		const MaVec3d& T = pTan1[ i ];

		Tangent = ( T - N * N.dot( T ) );
		Tangent.normalise();

		// Calculate handedness
		BcF32 W = ( N.cross( T ).dot( pTan2[ i ] ) < 0.0f ) ? -1.0f : 1.0f;

		if ( W < 0.0f )
		{
			Tangent = -Tangent;
		}

		Vert.bTangent_ = BcTrue;
		Vert.Tangent_ = Tangent;

		// Validate, and create a dummy value.
		BcF32 Mag = Tangent.magnitude(); 
		if( BcAbs( Mag - 1.0f ) > 0.0001f )
		{
			Vert.Tangent_.set( 0.0f, 0.0f, 0.0f );
		}
	}

	delete[] pTan1;
}

//////////////////////////////////////////////////////////////////////////
// flipIndices
void MdlMesh::flipIndices()
{
	for( size_t i = 0; i < aIndices_.size(); i += 3 )
	{
		MdlIndex Temp = aIndices_[ i ];
		aIndices_[ i ] = aIndices_[ i + 2 ];
		aIndices_[ i + 2 ] = Temp;
	}
}

//////////////////////////////////////////////////////////////////////////
// findAABB
MaAABB MdlMesh::findAABB() const
{
	MaAABB MeshBounds;

	for( size_t VertIdx = 0; VertIdx < aVertices_.size(); ++VertIdx )
	{
		MeshBounds.expandBy( aVertices_[ VertIdx ].Position_ );
	}

	return MeshBounds;
}

//////////////////////////////////////////////////////////////////////////
// findBoneCount
size_t MdlMesh::findBoneCount() const
{
	std::map< size_t, BcBool > BoneMap;

	for( size_t VertIdx = 0; VertIdx < aVertices_.size(); ++VertIdx )
	{
		const MdlVertex& Vertex = aVertices_[ VertIdx ];
		for( size_t BoneIdx = 0; BoneIdx < 4; ++BoneIdx )
		{
			BcF32 Weight = Vertex.Weights_[ BoneIdx ];
			size_t Joint = Vertex.iJoints_[ BoneIdx ];
			if( Weight > 0.0f && Joint != BcErrorCode )
			{
				BoneMap[ Joint ] = BcTrue;
			}
		}
	}

	return static_cast< size_t >( BoneMap.size() );
}

//////////////////////////////////////////////////////////////////////////
// splitByMaterial
std::vector< MdlMesh >& MdlMesh::splitByMaterial()
{
	// Empty current submeshes.
	SubMeshes_.clear();
	
	// Create as many submeshes as we need.
	SubMeshes_.resize( nMaterials() );

	// Re-add all vertices for each material to the mesh.
	for( size_t iMaterial = 0; iMaterial < aMaterials_.size(); ++iMaterial )
	{
		// Cache submesh.
		MdlMesh& SubMesh = SubMeshes_[ iMaterial ];

		// Add material to mesh.
		SubMesh.addMaterial( material( iMaterial ) );

		// Add each index thats bound to this material to the new list.
		for( size_t iIndex = 0; iIndex < aIndices_.size(); ++iIndex )
		{
			if( aIndices_[ iIndex ].iMaterial_ == iMaterial )
			{
				// Add corresponding vertex.
				MdlIndex NewIndex;
				NewIndex.iVertex_ = SubMesh.addVertexShared( vertex( aIndices_[ iIndex ].iVertex_ ) );
				NewIndex.iMaterial_ = iMaterial;

				// Add new index.
				SubMesh.addIndex( NewIndex );
			}
		}

		BcAssert( SubMesh.nIndices() % 3 == 0 );
	}
	
	return SubMeshes_;
}

//////////////////////////////////////////////////////////////////////////
// splitIntoBonePalettes
std::vector< MdlMesh >& MdlMesh::splitIntoBonePalettes( BcU32 PaletteSize )
{
	// Empty current submeshes.
	SubMeshes_.clear();

	// Build a triangle array.
	MdlTriangleArray Triangles;
	size_t NoofTriangles = aIndices_.size() / 3;
	Triangles.reserve( NoofTriangles );
	for( size_t Idx = 0; Idx < NoofTriangles; ++Idx )
	{
		size_t StartIndex = Idx * 3;
		MdlTriangle Triangle = 
		{
			aVertices_[ aIndices_[ StartIndex + 0 ].iVertex_ ],
			aVertices_[ aIndices_[ StartIndex + 1 ].iVertex_ ],
			aVertices_[ aIndices_[ StartIndex + 2 ].iVertex_ ]
		};
		Triangles.push_back( Triangle );
	}

	while( Triangles.size() > 0 )
	{
		MdlBonePalette NewBonePalette( PaletteSize );
		MdlTriangleArray NewTriangles;
		NewTriangles.reserve( Triangles.size() );

		// Iterate over triangles, and add them to the bone palette if possible.
		for( MdlTriangleArray::iterator It( Triangles.begin() ); It != Triangles.end(); )
		{
			MdlTriangle& Triangle = (*It);

			// If we can add it, add it to the new triangles array + erase it.
			if( NewBonePalette.addTriangle( Triangle ) )
			{
				NewTriangles.push_back( Triangle );
				It = Triangles.erase( It );
			}
			else
			{
				++It;
			}
		}

		// Construct a new mesh for the triangles.
		MdlMesh NewMesh;
		NewMesh.bonePalette( NewBonePalette );

		// Add materials.
		BcAssert( aMaterials_.size() == 1 );
		for( BcU32 Idx = 0; Idx < aMaterials_.size(); ++Idx )
		{
			NewMesh.addMaterial( aMaterials_[ Idx ] );
		}

		// Add triangles.
		for( size_t Idx = 0; Idx < NewTriangles.size(); ++Idx )
		{
			const MdlTriangle& Triangle( NewTriangles[ Idx ] );
			
			for( size_t VertIdx = 0; VertIdx < 3; ++VertIdx )
			{
				MdlIndex Index;
				Index.iVertex_ = NewMesh.addVertexShared( Triangle.Vertex_[ VertIdx ] );
				Index.iMaterial_ = 0;
				NewMesh.addIndex( Index );
			}
		}

		BcAssert( NewMesh.findBoneCount() <= PaletteSize );

		SubMeshes_.push_back( NewMesh );
	}
	
	return SubMeshes_;
}

//////////////////////////////////////////////////////////////////////////
// flipCoordinateSpace
void MdlMesh::flipCoordinateSpace()
{
	for( BcU32 i = 0; i < aVertices_.size(); ++i )
	{
		MdlVertex& Vertex = aVertices_[ i ];

		Vertex.Position_.z( -Vertex.Position_.z() );
		Vertex.Normal_.z( -Vertex.Normal_.z() );
		Vertex.Tangent_.z( -Vertex.Tangent_.z() );
		Vertex.UV_.y( -Vertex.UV_.y() );			// should be external.
	}

	// Flip indices too.
	flipIndices();
}
