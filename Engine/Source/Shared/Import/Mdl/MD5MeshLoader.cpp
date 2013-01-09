/**************************************************************************
*
* File:		MD5MeshLoader.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "MD5MeshLoader.h"

#include "Base/BcDebug.h"
#include "Base/BcFile.h"
#include "Base/BcString.h"
#include "Base/BcQuat.h"

#include "MdlNode.h"
#include "MdlMesh.h"

//////////////////////////////////////////////////////////////////////////
// Constructor
MD5MeshLoader::MD5MeshLoader()
{

}

//////////////////////////////////////////////////////////////////////////
// Destructor
MD5MeshLoader::~MD5MeshLoader()
{

}

//////////////////////////////////////////////////////////////////////////
// load
MdlNode* MD5MeshLoader::load( const BcChar* FileName, const BcChar* NodeName )
{
	BcFile File;
	BcBool Ret;

	ParseMode_ = PM_MAIN;

	BcU32 iJoint = 0;
	BcU32 iMesh = 0;
	BcU32 iVert = 0;
	BcU32 iInd = 0;
	BcU32 iWeight = 0;

	if( File.open( FileName ) == BcFalse )
	{
		return NULL;
	}

	BcChar Buffer[4096];
	BcChar Command[4096];
	BcChar* pBuffer;

	// Begin the parsage.
	while( !File.eof() )
	{
		// Parse a line	without	the	spaces at the start	or tabs.
		{
			pBuffer	= &Buffer[0];

			BcChar TheChar = 0;
			BcBool bAtSentence = BcFalse;
			while( TheChar != 10 &&	!File.eof()	)
			{
				File.read( &TheChar, 1 );

				if ( TheChar != '\t' )
				{
					bAtSentence	= BcTrue;
				}

				if ( TheChar != 10 &&
				     TheChar != 13 &&
				     bAtSentence )
				{
					*pBuffer = TheChar;
					++pBuffer;
					Ret	= ( pBuffer < ( Buffer + 4096 ) );
					BcAssert( Ret );
					if( !Ret )
					{
						return NULL;
					}
				}
			}

			// Terminate it.
			*pBuffer = 0;
			++pBuffer;
		}

		sscanf( Buffer, "%s", Command );

		if ( BcStrLength( Buffer ) == 0 )
		{
			Command[ 0 ] = 0;
		}

		switch( ParseMode_ )
		{
		case PM_MAIN:
			{
				if( BcStrCompare( "numJoints", Command ) )
				{
					sscanf( Buffer, "numJoints %u", &nJoints_ );
					
					pJoints_ = new MD5_Joint[ nJoints_ ]; 
				}
				else if( BcStrCompare( "numMeshes", Command ) )
				{
					sscanf( Buffer, "numMeshes %u", &nMeshes_ );
					
					pMeshes_ = new MD5_Mesh[ nMeshes_ ]; 
				}
				else if( BcStrCompare( "joints", Command ) )
				{
					ParseMode_ = PM_JOINTS;
				}
				else if( BcStrCompare( "mesh", Command ) )
				{
					ParseMode_ = PM_MESH;

					iVert = 0;
					iInd = 0;
					iWeight = 0;
				}
			}
			break;

		case PM_JOINTS:
			{
				if( BcStrCompare( "}", Command ) )
				{
					ParseMode_ = PM_MAIN;
				}
				else
				{
					// Parse a joint off.
					BcAssert( iJoint < nJoints_ );
					MD5_Joint* pJoint = &pJoints_[ iJoint ];
					sscanf( Buffer, "%s %i ( %f %f %f ) ( %f %f %f )", pJoint->Name_, &pJoint->ParentID_, &pJoint->TX_, &pJoint->TY_, &pJoint->TZ_, &pJoint->QX_, &pJoint->QY_, &pJoint->QZ_ );
					++iJoint;
				}
			}
			break;

		case PM_MESH:
			{
				if( BcStrCompare( "}", Command ) )
				{
					++iMesh;
					ParseMode_ = PM_MAIN;
				}
				else
				{
					BcAssert( iMesh < nMeshes_ );
					MD5_Mesh* pMesh = &pMeshes_[ iMesh ];

					if ( BcStrCompare( "shader", Command ) )
					{
						sscanf( Buffer, "shader \"%s", pMesh->Shader_ );
						BcU32 i = BcStrLength( pMesh->Shader_ ) - 1;
						pMesh->Shader_[ i ] = '\0';
					}
					else if ( BcStrCompare( "numverts", Command ) )
					{
						sscanf( Buffer, "numverts %u", &pMesh->nVerts_ );
						pMesh->pVerts_ = new MD5_Vert[ pMesh->nVerts_ ];
					}
					else if ( BcStrCompare( "vert", Command ) )
					{
						// Parse a vert off
						BcAssert( iVert < pMesh->nVerts_ );
						MD5_Vert* pVert = &pMesh->pVerts_[ iVert ];
						sscanf( Buffer, "vert %i ( %f %f ) %i %i", &pVert->Index_, &pVert->U_, &pVert->V_, &pVert->WeightIndex_, &pVert->nWeights_ );
						++iVert;
					}
					else if ( BcStrCompare( "numtris", Command ) )
					{
						sscanf( Buffer, "numtris %u", &pMesh->nIndices_ );
						pMesh->nIndices_ *= 3;
						pMesh->pIndices_ = new BcU32[ pMesh->nIndices_ ];
					}
					else if ( BcStrCompare( "tri", Command ) )
					{
						// Parse a vert off
						BcAssert( iInd < pMesh->nIndices_ );
						BcU32 Index;
						BcU32* pIndex = &pMesh->pIndices_[ iInd ];
						sscanf( Buffer, "tri %i %i %i %i", &Index, &pIndex[0], &pIndex[1], &pIndex[2] );
						iInd += 3;
					}
					else if ( BcStrCompare( "numweights", Command ) )
					{
						sscanf( Buffer, "numweights %u", &pMesh->nWeights_ );
						pMesh->pWeights_ = new MD5_Weight[ pMesh->nWeights_ ];
					}
					else if ( BcStrCompare( "weight", Command ) )
					{
						// Parse a vert off
						BcAssert( iWeight < pMesh->nWeights_ );
						MD5_Weight* pWeight = &pMesh->pWeights_[ iWeight ];
						sscanf( Buffer, "weight %i %i %f ( %f %f %f )", &pWeight->Index_, &pWeight->JointID_, &pWeight->Weight_, &pWeight->X_, &pWeight->Y_, &pWeight->Z_ );
						++iWeight;
					}
				}
			}
			break;
		}
	}

	// Now that we are loaded we can build the node graph.
	MdlNode* pRootNode = new MdlNode();

	pRootNode->name( NodeName );

	for( BcU32 i = 0; i < nMeshes_; ++i )
	{
		buildBindPose( pRootNode, i );
	}

	// Parent joints up.
	//if( nJoints_ > 1 )
	{
		for( BcU32 i = 0; i < nJoints_; ++i )
		{
			MD5_Joint* pJoint = &pJoints_[ i ];
			MdlNode* pNode = new MdlNode();

			pNode->name( pJoint->Name_ );
			pNode->type( eNT_JOINT );

			// Build the transform and inverse bind pose.
			BcQuat JointRot( pJoint->QX_, pJoint->QY_, pJoint->QZ_, 0.0f );
			JointRot.calcFromXYZ();
			BcMat4d JointRotation;
			BcMat4d JointTranslate;
			BcMat4d JointTransform;
			BcMat4d InverseBindpose;
			JointRot.asMatrix4d( JointRotation );
			JointTranslate.identity();
			JointTranslate.row3( BcVec4d( pJoint->TX_, pJoint->TY_, pJoint->TZ_, 1.0f ) );

			JointTransform = JointRotation * JointTranslate;
			InverseBindpose = JointTransform;
			InverseBindpose.inverse();

			pNode->absoluteTransform( JointTransform );
			pNode->inverseBindpose( InverseBindpose );

			if( pJoint->ParentID_ != BcErrorCode )
			{
				pRootNode->parentNode( pNode, pJoints_[ pJoint->ParentID_ ].Name_ );
			}
			else
			{
				// Parent to root.
				pRootNode->parentNode( pNode, pRootNode->name() );
			}
		}
	}

	// Cleanup
	nJoints_ = 0;
	delete [] pJoints_;
	pJoints_ = NULL;

	for( BcU32 i = 0; i < nMeshes_; ++i )
	{
		delete [] pMeshes_[ i ].pIndices_;
		delete [] pMeshes_[ i ].pVerts_;
		delete [] pMeshes_[ i ].pWeights_;
	}

	nMeshes_ = 0;
	delete [] pMeshes_;
	pMeshes_ = NULL;

	BcMat4d RootTransform;
#if 1
	// Convert from Z up to Y up & right hand to left hand.
	RootTransform.row0( BcVec4d(  1.0f,  0.0f,  0.0f,  0.0f ) );
	RootTransform.row1( BcVec4d(  0.0f,  0.0f,  1.0f,  0.0f ) );
	RootTransform.row2( BcVec4d(  0.0f, -1.0f,  0.0f,  0.0f ) );
	RootTransform.row3( BcVec4d(  0.0f,  0.0f,  0.0f,  1.0f ) );
#endif
	pRootNode->makeRelativeTransform( RootTransform );

	return pRootNode;
}

//////////////////////////////////////////////////////////////////////////
// buildBindPose
void MD5MeshLoader::buildBindPose( MdlNode* pNode, BcU32 iMesh )
{
	// Skin the mesh.
	MD5_Joint* pJoints = pJoint( 0 );
	MD5_Mesh* pMeshes = pMesh( iMesh );

	// This node is a mesh if we have 1 joint, skin if we have more.
	MdlMesh* pMesh = NULL;
	if( nJoints_ == 1 )
	{
		pNode->type( eNT_MESH );
		pMesh = pNode->pMeshObject();
		
	}
	else
	{
		pNode->type( eNT_SKIN );
		pMesh = pNode->pSkinObject();
	}
	// Add material.
	MdlMaterial Material;
	Material.default3d();
	Material.Name_ = pMeshes->Shader_;
	BcU32 iMaterial = pMesh->addMaterial( Material );

	// Add indices.
	BcU32 BaseIndex = pMesh->nVertices();
	for( BcU32 i = 0; i < pMeshes->nIndices_; ++i )
	{
		MdlIndex Index;
		Index.iMaterial_ = iMaterial;
		Index.iVertex_ = pMeshes->pIndices_[ i ] + BaseIndex;
		pMesh->addIndex( Index );
	}

	BcVec3d WeightPos;
	for ( BcU32 i = 0; i < pMeshes->nVerts_; ++i )
	{
		MD5_Vert* pMD5Vert = &pMeshes->pVerts_[ i ];
		BcVec3d Position( 0.0f, 0.0f, 0.0f );

		// Setup vertex.
		MdlVertex Vert;

		Vert.bUV_ = BcTrue;
		Vert.UV_.x( pMD5Vert->U_ );
		Vert.UV_.y( pMD5Vert->V_ );

		for ( BcU32 j = 0; j < 4; ++j )
		{
			Vert.Weights_[ j ] = 0.0f;
			Vert.iJoints_[ j ]  = BcErrorCode;
		}

		// Create weights.
		Vert.nWeights_ = 0;
		for ( BcU32 j = 0; j < pMD5Vert->nWeights_; ++j )
		{
			if ( j > 3 ) 
			{
				break;
			}

			BcU32 WeightIndex = pMD5Vert->WeightIndex_ + j;
			MD5_Weight* pMD5Weight = &pMeshes->pWeights_[ WeightIndex ];

			// Setup vertex indices and weights:
			Vert.iJoints_[ j ] = pMD5Weight->JointID_;
			Vert.Weights_[ j ] = pMD5Weight->Weight_;
			Vert.nWeights_++;
		}

		// Normalise weights.

		BcF32 WeightTotal = 0.0f;

		for( BcU32 j = 0; j < Vert.nWeights_; ++j )
		{
			WeightTotal += Vert.Weights_[ j ];
		}

		for( BcU32 j = 0; j < Vert.nWeights_; ++j )
		{
			Vert.Weights_[ j ] = Vert.Weights_[ j ] / WeightTotal;
		}
		
		// Generate bind pose.
		BcVec3d VertexWeights[ 4 ];
		BcVec3d JointPositions[ 4 ];
		BcQuat JointQuats[ 4 ];
		for ( BcU32 j = 0; j < Vert.nWeights_; ++j )
		{
			BcU32 WeightIndex = pMD5Vert->WeightIndex_ + j;
			MD5_Weight* pMD5Weight = &pMeshes->pWeights_[ WeightIndex ];

			// Fix up a joint.
			MD5_Joint* pJoint = &pJoints[ pMD5Weight->JointID_ ];
			BcVec3d JointTran( pJoint->TX_, pJoint->TY_, pJoint->TZ_ );
			BcQuat JointRot( pJoint->QX_, pJoint->QY_, pJoint->QZ_, 0.0f );
			JointRot.calcFromXYZ();

			VertexWeights[ j ] = BcVec3d( pMD5Weight->X_, pMD5Weight->Y_, pMD5Weight->Z_ );
			JointPositions[ j ] = JointTran;
			JointQuats[ j ] = JointRot;
		}

		for ( BcU32 j = 0; j < Vert.nWeights_; ++j )
		{
			BcVec3d WeightPos( VertexWeights[ j ] );
			const BcVec3d& JointTran( JointPositions[ j ] );
			const BcQuat& JointRot( JointQuats[ j ] );

			JointRot.rotateVector( WeightPos );
			Position += ( WeightPos + JointTran ) * Vert.Weights_[ j ];
		}


		Vert.bPosition_ = BcTrue;
		Vert.Position_ = Position;

		pMesh->addVertex( Vert );
	}

	pMesh->sortIndicesByMaterial();
	pMesh->buildNormals();
	pMesh->buildTangents();

	// Setup AABB to be larger than the skin to account for motion.
	BcAABB AABB = pMesh->findAABB();

	AABB.min( AABB.min() * 1.5f );
	AABB.max( AABB.max() * 1.5f );

	pNode->aabb( AABB );

	// TODO: Refine into bone palettes properly.
}

//////////////////////////////////////////////////////////////////////////
// buildTangents
