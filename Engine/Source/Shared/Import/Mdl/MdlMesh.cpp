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

//////////////////////////////////////////////////////////////////////////
// Ctor
MdlMesh::MdlMesh()
{

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
BcU32 MdlMesh::addVertex( const MdlVertex& Vertex )
{
	aVertices_.push_back( Vertex );
	return (BcU32)( aVertices_.size() - 1 );
}

//////////////////////////////////////////////////////////////////////////
// addMaterial
BcU32 MdlMesh::addMaterial( const MdlMaterial& Material )
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
BcU32 MdlMesh::addVertexShared( const MdlVertex& Vertex )
{
	// Go through all our vertices, and if we find a matching one,
	// return the index to it, else just add this vert and return
	// the index to it.
	BcU32 iVertex = 0;
	BcBool bFoundVertex = BcFalse;

	for( BcU32 i = 0; i < aVertices_.size(); ++i )
	{
		// If we find the vertex, we can bail.
		if( compareVertices( Vertex, aVertices_[ i ] ) )
		{
			iVertex = i;
			bFoundVertex = BcTrue;
			break;
		}
	}

	// If we didn't find a vertex then add it.
	if( bFoundVertex == BcFalse )
	{
		addVertex( Vertex );
		iVertex = BcU32( aVertices_.size() ) - 1;
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
BcBool MdlMesh::materialIndexCount( BcU32 iMaterial, BcU32& iFirst, BcU32& nIndices )
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
	for( BcU32 i = iFirst; i < aIndices_.size(); ++i )
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
// countJointsUsed
BcU32 MdlMesh::countJointsUsed( BcU32 iMaterial )
{
	BcU32 nJoints = 0;

	MdlVertex CurrVertex;

	for( BcU32 i = 0; i < aIndices_.size(); ++i )
	{
		if( aIndices_[ i ].iMaterial_ == iMaterial )
		{
			CurrVertex = aVertices_[ aIndices_[ i ].iVertex_ ];

			for( BcU32 j = 0; j < 4; ++j )
			{
				if( CurrVertex.iJoints_[ j ] > nJoints )
				{
					nJoints = CurrVertex.iJoints_[ j ];
				}
			}
		}
	}

	return nJoints + 1;
}

//////////////////////////////////////////////////////////////////////////
// bakeTransform
void MdlMesh::bakeTransform( const BcMat4d& Transform )
{
	BcMat4d NrmTransform = Transform;
	NrmTransform.translation( BcVec3d( 0.0f, 0.0f, 0.0f ) );
	for( BcU32 i = 0; i < aVertices_.size(); ++i )
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
	for ( BcU32 i = 0; i < ( aIndices_.size() / 3 ); ++i )
	{
		BcU32 TA = aIndices_[ ( i * 3 ) + 0 ].iVertex_;
		BcU32 TB = aIndices_[ ( i * 3 ) + 1 ].iVertex_;
		BcU32 TC = aIndices_[ ( i * 3 ) + 2 ].iVertex_;

		MdlVertex& VertA = aVertices_[ TA ];
		MdlVertex& VertB = aVertices_[ TB ];
		MdlVertex& VertC = aVertices_[ TC ];

		BcVec3d VertPosA( VertA.Position_.x(), VertA.Position_.y(), VertA.Position_.z() );
		BcVec3d VertPosB( VertB.Position_.x(), VertB.Position_.y(), VertB.Position_.z() );
		BcVec3d VertPosC( VertC.Position_.x(), VertC.Position_.y(), VertC.Position_.z() );

		BcVec3d Normal = ( VertPosA - VertPosB ).cross( ( VertPosB - VertPosC ) );

		VertA.Normal_ += Normal;
		VertB.Normal_ += Normal;
		VertC.Normal_ += Normal;
	}

	for ( BcU32 i = 0; i < aVertices_.size(); ++i )
	{
		MdlVertex& Vert = aVertices_[ i ];
		Vert.bNormal_ = BcTrue;
		Vert.Normal_.normalise();

		BcReal Mag = Vert.Normal_.magnitude(); 
		BcAssert( BcAbs( Mag - 1.0f ) < 0.00001f );
	}
}

//////////////////////////////////////////////////////////////////////////
// buildTangents
void MdlMesh::buildTangents()
{
	BcVec3d* pTan1 = new BcVec3d[ aVertices_.size() * 2 ];
	BcVec3d* pTan2 = pTan1 + aVertices_.size();

	memset( pTan1, 0, sizeof( BcVec3d ) * aVertices_.size() * 2 );

	for ( BcU32 i = 0; i < ( aIndices_.size() / 3 ); ++i )
	{
		BcU32 TA = aIndices_[ ( i * 3 ) + 0 ].iVertex_;
		BcU32 TB = aIndices_[ ( i * 3 ) + 1 ].iVertex_;
		BcU32 TC = aIndices_[ ( i * 3 ) + 2 ].iVertex_;

		MdlVertex& VertA = aVertices_[ TA ];
		MdlVertex& VertB = aVertices_[ TB ];
		MdlVertex& VertC = aVertices_[ TC ];

		BcVec3d VertPosA = VertA.Position_;
		BcVec3d VertPosB = VertB.Position_;
		BcVec3d VertPosC = VertC.Position_;

		BcVec2d VertUVA = VertA.UV_;
		BcVec2d VertUVB = VertB.UV_;
		BcVec2d VertUVC = VertC.UV_;

		BcReal X1 = VertPosB.x() - VertPosA.x();
		BcReal X2 = VertPosC.x() - VertPosA.x();
		BcReal Y1 = VertPosB.y() - VertPosA.y();
		BcReal Y2 = VertPosC.y() - VertPosA.y();
		BcReal Z1 = VertPosB.z() - VertPosA.z();
		BcReal Z2 = VertPosC.z() - VertPosA.z();

		BcReal S1 = VertUVB.x() - VertUVA.x();
		BcReal S2 = VertUVC.x() - VertUVA.x();
		BcReal T1 = VertUVB.y() - VertUVA.y();
		BcReal T2 = VertUVC.y() - VertUVA.y();

		BcReal InvR = ( S1 * T2 - S2 * T1 );
		BcReal R = 1.0f / InvR;

		// Validation so it doesn't break everything, just set to a dummy value.
		if( BcAbs( InvR ) < 1e6f )
		{
			R = 0.0f;
		}

		BcVec3d SDir( ( T2 * X1 - T1 * X2 ) * R, ( T2 * Y1 - T1 * Y2 ) * R, ( T2 * Z1 - T1 * Z2 ) * R );
		BcVec3d TDir( ( S1 * X2 - S2 * X1 ) * R, ( S1 * Y2 - S2 * Y1 ) * R, ( S1 * Z2 - S2 * Z1 ) * R );

		pTan1[ TA ] += SDir;
		pTan1[ TB ] += SDir;
		pTan1[ TC ] += SDir;

		pTan2[ TA ] += TDir;
		pTan2[ TB ] += TDir;
		pTan2[ TC ] += TDir;
	}

	for ( BcU32 i = 0; i < aVertices_.size(); ++i )
	{
		MdlVertex& Vert = aVertices_[ i ];
		BcVec3d Tangent;

		const BcVec3d N = Vert.Normal_;
		const BcVec3d& T = pTan1[ i ];

		Tangent = ( T - N * N.dot( T ) );
		Tangent.normalise();

		// Calculate handedness
		BcReal W = ( N.cross( T ).dot( pTan2[ i ] ) < 0.0f ) ? -1.0f : 1.0f;

		if ( W < 0.0f )
		{
			Tangent = -Tangent;
		}

		Vert.bTangent_ = BcTrue;
		Vert.Tangent_ = Tangent;

		// Validate, and create a dummy value.
		BcReal Mag = Tangent.magnitude(); 
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
	for( BcU32 i = 0; i < aIndices_.size(); i += 3 )
	{
		MdlIndex Temp = aIndices_[ i ];
		aIndices_[ i ] = aIndices_[ i + 2 ];
		aIndices_[ i + 2 ] = Temp;
	}
}

//////////////////////////////////////////////////////////////////////////
// findAABB
BcAABB MdlMesh::findAABB() const
{
	BcAABB MeshBounds;

	for( BcU32 i = 0; i < aVertices_.size(); ++i )
	{
		MeshBounds.expandBy( aVertices_[ i ].Position_ );
	}

	return MeshBounds;
}

//////////////////////////////////////////////////////////////////////////
// splitByMaterial
const std::vector< MdlMesh >& MdlMesh::splitByMaterial()
{
	// Empty current submeshes.
	SubMeshes_.clear();
	
	// Create as many submeshes as we need.
	SubMeshes_.resize( nMaterials() );

	// Re-add all vertices for each material to the mesh.
	for( BcU32 iMaterial = 0; iMaterial < aMaterials_.size(); ++iMaterial )
	{
		// Cache submesh.
		MdlMesh& SubMesh = SubMeshes_[ iMaterial ];

		// Add material to mesh.
		SubMesh.addMaterial( material( iMaterial ) );

		// Add each index thats bound to this material to the new list.
		for( BcU32 iIndex = 0; iIndex < aIndices_.size(); ++iIndex )
		{
			if( aIndices_[ iIndex ].iMaterial_ == iMaterial )
			{
				// Add corresponding vertex.
				MdlIndex NewIndex;
				NewIndex.iVertex_ = SubMesh.addVertex( vertex( aIndices_[ iIndex ].iVertex_ ) );
				NewIndex.iMaterial_ = iMaterial;

				// Add new index.
				SubMesh.addIndex( NewIndex );
			}
		}
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
