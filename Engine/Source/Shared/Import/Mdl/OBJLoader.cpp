/**************************************************************************
*
* File:		OBJLoader.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "OBJLoader.h"

#include "Mdl.h"

#include "Base/BcFile.h"
#include "Base/BcString.h"
#include "Base/BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
OBJLoader::OBJLoader()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
OBJLoader::~OBJLoader()
{
	
}

//////////////////////////////////////////////////////////////////////////
// load
MdlNode* OBJLoader::load( const BcChar* FileName, const BcChar* NodeName )
{
	BcFile File;
	MdlNode* pNode = NULL;
	BcChar LineBuffer[ 1024 ];
	BcChar CommandBuffer[ 1024 ];
	
	if( File.open( FileName ) )
	{
		// Create mesh node.
		pNode = new MdlNode();
		pNode->type( eNT_MESH );
		pNode->name( NodeName );
		MdlMesh* pMesh = pNode->pMeshObject();
		MdlMaterial Material;
		BcU32 MaterialIdx = BcErrorCode;

		// Read lines in until EOF.
		while( !File.eof() )
		{
			// Read line.
			File.readLine( LineBuffer, sizeof( LineBuffer ) );

			// Parse out command.
			BcMemZero( CommandBuffer, sizeof( CommandBuffer ) );
			BcSScanf( LineBuffer, "%s ", CommandBuffer );

			// Handle them.
			if( BcStrCompare( CommandBuffer, "mtllib" ) )
			{
				// Load mtllib.
				//BcChar MtlFile[ 1024 ];
				//BcSScanf( LineBuffer, "%s %s", CommandBuffer, MtlFile );
				//MdlMaterial Material = loadMtl( MtlFile );
			}
			else if( BcStrCompare( CommandBuffer, "v" ) )
			{
				// Vertex position.
				BcF32 X, Y, Z;
				BcSScanf( LineBuffer, "%s %f %f %f", CommandBuffer, &X, &Y, &Z );
				Positions_.push_back( BcVec3d( -X, Y, Z ) );
			}
			else if( BcStrCompare( CommandBuffer, "vn" ) )
			{
				// Vertex normal.
				BcF32 X, Y, Z;
				BcSScanf( LineBuffer, "%s %f %f %f", CommandBuffer, &X, &Y, &Z );
				Normals_.push_back( BcVec3d( -X, Y, Z ) );
			}
			else if( BcStrCompare( CommandBuffer, "vt" ) )
			{
				// Vertex texcoord.
				BcF32 U, V;
				BcSScanf( LineBuffer, "%s %f %f", CommandBuffer, &U, &V );
				TexCoords_.push_back( BcVec2d( U, V ) );
			}
			else if( BcStrCompare( CommandBuffer, "usemtl" ) )
			{
				// Use material.
				BcChar NameBuffer[ 1024 ];
				BcMemSet( NameBuffer, 0, sizeof( NameBuffer ) );
				BcSScanf( LineBuffer, "%s %s", CommandBuffer, NameBuffer );
				Material.Name_ = NameBuffer;
				MaterialIdx = pMesh->addMaterial( Material );
			}
			else if( BcStrCompare( CommandBuffer, "f" ) )
			{
				// Face.
				BcU32 P[3] = { BcErrorCode, BcErrorCode, BcErrorCode };
				BcU32 N[3] = { BcErrorCode, BcErrorCode, BcErrorCode };
				BcU32 T[3] = { BcErrorCode, BcErrorCode, BcErrorCode };

				// Parse face line.
				BcU32 NoofSlashes = BcStrCountChars( LineBuffer, '/' );
				switch( NoofSlashes )
				{
				case 0:
					BcSScanf( LineBuffer, "%s %u %u %u", CommandBuffer, &P[0], &P[1], &P[2] );
					break;
				case 3:
					BcSScanf( LineBuffer, "%s %u/%u %u/%u %u/%u", CommandBuffer, &P[0], &T[0], &P[1], &T[1], &P[2], &T[2] );
					break;
				case 6:
					BcSScanf( LineBuffer, "%s %u/%u/%u %u/%u/%u %u/%u/%u", CommandBuffer, &P[0], &T[0], &N[0], &P[1], &T[1], &N[1], &P[2], &T[2], &N[2] );
					if( T[0] == BcErrorCode )
					{
						BcSScanf( LineBuffer, "%s %u//%u %u//%u %u//%u", CommandBuffer, &P[0], &N[0], &P[1], &N[1], &P[2], &N[2] );
					}
					break;
				default:
					BcPrintf( "OBJLoader: Only supports triangulated faces (for now..)!\n" );
					return NULL;
					break;
				}

				// Add vertex to mesh.
				MdlVertex VertA, VertB, VertC;
				VertA.bPosition_ = BcTrue;
				VertB.bPosition_ = BcTrue;
				VertC.bPosition_ = BcTrue;
				VertA.Position_ = Positions_[ P[0] - 1 ];
				VertB.Position_ = Positions_[ P[1] - 1 ];
				VertC.Position_ = Positions_[ P[2] - 1 ];
						
				if( T[0] != BcErrorCode )
				{
					VertA.bUV_ = BcTrue;
					VertB.bUV_ = BcTrue;
					VertC.bUV_ = BcTrue;
					VertA.UV_ = TexCoords_[ T[0] - 1 ];
					VertB.UV_ = TexCoords_[ T[1] - 1 ];
					VertC.UV_ = TexCoords_[ T[2] - 1 ];

					VertA.UV_ = BcVec2d( VertA.UV_.x(), -VertA.UV_.y() );
					VertB.UV_ = BcVec2d( VertB.UV_.x(), -VertB.UV_.y() );
					VertC.UV_ = BcVec2d( VertC.UV_.x(), -VertC.UV_.y() );
				}

				if( N[0] != BcErrorCode )
				{
					VertA.bNormal_ = BcTrue;
					VertB.bNormal_ = BcTrue;
					VertC.bNormal_ = BcTrue;
					VertA.Normal_ = Normals_[ N[0] - 1 ];
					VertB.Normal_ = Normals_[ N[1] - 1 ];
					VertC.Normal_ = Normals_[ N[2] - 1 ];
				}

				MdlIndex IdxA =
				{
					pMesh->addVertex( VertA ),
					MaterialIdx
				};

				MdlIndex IdxB =
				{
					pMesh->addVertex( VertB ),
					MaterialIdx
				};

				MdlIndex IdxC =
				{
					pMesh->addVertex( VertC ),
					MaterialIdx
				};

				// Add triangle.
				pMesh->addIndex( IdxA );
				pMesh->addIndex( IdxB );
				pMesh->addIndex( IdxC );
			}
			else if( BcStrCompare( CommandBuffer, "s" ) )
			{
				// Smoothing group.
			}
			else if( BcStrCompare( CommandBuffer, "g" ) )
			{
				// Group.
			}
			else if( BcStrCompare( CommandBuffer, "o" ) )
			{
				// Object.
			}
		}

		pMesh->buildNormals();
		pMesh->buildTangents();
	}

	return pNode;
}
