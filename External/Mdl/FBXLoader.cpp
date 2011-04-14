/**************************************************************************
*
* File:		FBXLoader.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		FBX Loader.
*		
*
*
* 
**************************************************************************/

#include "FBXLoader.h"

#include "PaUtility.h"

#include "MdlMesh.h"

static KFbxSdkManager* gpSdkManager_ = NULL;
static KFbxScene*      gpScene_     = NULL;

//////////////////////////////////////////////////////////////////////////
// Utility.
BcMat4d Mat4dFromFBX( KFbxXMatrix& Source )
{
	BcMat4d Dest;

	KFbxVector4 Row0 = Source.GetRow( 0 );
	KFbxVector4 Row1 = Source.GetRow( 1 );
	KFbxVector4 Row2 = Source.GetRow( 2 );
	KFbxVector4 Row3 = Source.GetRow( 3 );

	Dest.row0( BcVec4d(  Row0[0],  Row0[1],  Row0[2], Row0[3] ) );
	Dest.row1( BcVec4d(  Row1[0],  Row1[1],  Row1[2], Row1[3] ) );
	Dest.row2( BcVec4d(  Row2[0],  Row2[1],  Row2[2], Row2[3] ) );
	Dest.row3( BcVec4d(  Row3[0],  Row3[1],  Row3[2], Row3[3] ) );
	
	return Dest;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
FBXLoader::FBXLoader()
{
	// Setup FBX.
	gpSdkManager_ = KFbxSdkManager::Create();
	BcAssert( gpSdkManager_ != NULL );

}

//////////////////////////////////////////////////////////////////////////
// Dtor
FBXLoader::~FBXLoader()
{
	// Free settings
	IOSREF.FreeIOSettings();

	//
	gpSdkManager_->Destroy();
	gpSdkManager_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// load
MdlNode* FBXLoader::load( const BcChar* FileName, const BcChar* RootName )
{
	// Setup Scene.
	gpScene_ = KFbxScene::Create( gpSdkManager_, RootName );

	KFbxStreamOptionsFbxReader *lImportOptions = KFbxStreamOptionsFbxReader::Create( gpSdkManager_, "" );

	// Create an importer.
	KFbxImporter* lImporter = KFbxImporter::Create( gpSdkManager_, "" );

	int lFileFormat = -1;
	bool lStatus;
	bool lImportStatus = lImporter->Initialize( FileName );

	if( !lImportStatus )
	{
		// Destroy the import options
		if(lImportOptions)
		{
			lImportOptions->Destroy();
			lImportOptions = NULL;
		}

		// Destroy the importer
		lImporter->Destroy();
		
		throw PaException( "Error initialising FBX importer." );

		return NULL;
	}

	if (lImporter->IsFBX())
	{
		// Set the import states. By default, the import states are always set to 
		// true. The code below shows how to change these states.
		//lImportOptions->SetOption(KFBXSTREAMOPT_FBX_MATERIAL,        true);
		//lImportOptions->SetOption(KFBXSTREAMOPT_FBX_TEXTURE,         true);
		//lImportOptions->SetOption(KFBXSTREAMOPT_FBX_LINK,            true);
		//lImportOptions->SetOption(KFBXSTREAMOPT_FBX_SHAPE,           true);
		//lImportOptions->SetOption(KFBXSTREAMOPT_FBX_GOBO,            true);
		//lImportOptions->SetOption(KFBXSTREAMOPT_FBX_ANIMATION,       true);
		//lImportOptions->SetOption(KFBXSTREAMOPT_FBX_GLOBAL_SETTINGS, true);
	}
	
	lStatus = lImporter->Import( gpScene_, lImportOptions );
	
	if( !lStatus )
	{
		throw PaException( "Error importing." );
	}
	
	//
	MdlNode* pRootNode = exportNode( gpScene_->GetRootNode() );
	
	// Set root name.
	pRootNode->name( RootName );

	// Flip coordinate space.
	pRootNode->flipCoordinateSpace();
	
	//
	BcMat4d Conversion;
	pRootNode->makeRelativeTransform( Conversion );
	
	//
	return pRootNode;
}

//////////////////////////////////////////////////////////////////////////
// exportNode
MdlNode* FBXLoader::exportNode( KFbxNode* pFBXNode )
{
	MdlNode* pNode = new MdlNode();

	// Set node.
	pNode->name( pFBXNode->GetName() );

	// Get transform.
	KTime Time;
	Time.Set( 0 );

	KFbxXMatrix Transform = pFBXNode->GetGlobalFromCurrentTake( Time );
	pNode->absoluteTransform( Mat4dFromFBX( Transform ) );

	// Extract information.
	KFbxNodeAttribute* lNodeAttribute = pFBXNode->GetNodeAttribute();

	if( lNodeAttribute != NULL )
	{
		switch( lNodeAttribute->GetAttributeType() )
		{
		case KFbxNodeAttribute::eMESH:
			processMesh( pFBXNode, pNode );
			break;

		case KFbxNodeAttribute::eLIGHT:
			processLight( pFBXNode, pNode );
			break;
		}
	}

	// Get children and parent.
	for( BcU32 iChild = 0; iChild < pFBXNode->GetChildCount(); ++iChild )
	{
		MdlNode* pChildNode = exportNode( pFBXNode->GetChild( iChild ) );

		pNode->parentNode( pChildNode, NULL );
	}
	
	return pNode;
}
	
//////////////////////////////////////////////////////////////////////////
// processMesh
void FBXLoader::processMesh( KFbxNode* pFBXNode, MdlNode* pExportNode )
{
	KFbxMesh* pMesh = (KFbxMesh*) pFBXNode->GetNodeAttribute();

	int lVertexCount = pMesh->GetControlPointsCount();

	// No vertex to draw.
	if ( lVertexCount == 0 )
	{
		return;
	}

	pExportNode->type( eNT_MESH );

	//if( strstr( "C_", pExportNode->name() ) )
	{
		pExportNode->type( eNT_COLMESH );
	}
	//
	MdlMesh* pExportMesh = pExportNode->pMeshObject();


	// Get materials.
	BcU32 nMaterials = pMesh->GetNode()->GetMaterialCount();
	for( BcU32 iMaterial = 0; iMaterial < nMaterials; ++iMaterial )
	{
		MdlMaterial Material;
		Material.default3d();
		Material.Name_ = PaUniqueString();

		// Extract materials for mesh.
		KFbxLayerElement::EMappingMode lMappingMode = KFbxLayerElement::eNONE;

		KFbxSurfaceMaterial* lSurfaceMaterial= KFbxCast <KFbxSurfaceMaterial>(pMesh->GetNode()->GetSrcObject(KFbxSurfaceMaterial::ClassId, iMaterial));
		if(lSurfaceMaterial)
		{
			KFbxProperty lAmbientProperty;
			KFbxProperty lDiffuseProperty;
			KFbxProperty lSpecularProperty;
			KFbxProperty lSpecularPowerProperty;

			// Grab ambient layer.
			{
				lAmbientProperty = lSurfaceMaterial->FindProperty(KFbxSurfaceMaterial::sAmbient);

				// Ambient Colour.
				if(lAmbientProperty.IsValid())
				{   
					KFbxColor Color;
					lAmbientProperty.Get( &Color, eDOUBLE4 );
					Material.AmbientColour_.set( Color.mRed, Color.mGreen, Color.mBlue, Color.mAlpha );
				}
			}

			// Grab diffuse layer.
			{
				lDiffuseProperty = lSurfaceMaterial->FindProperty(KFbxSurfaceMaterial::sDiffuse);

				// Diffuse Colour.
				if(lDiffuseProperty.IsValid())
				{   
					KFbxColor Color;
					lDiffuseProperty.Get( &Color, eDOUBLE4 );
					Material.DiffuseColour_.set( Color.mRed, Color.mGreen, Color.mBlue, Color.mAlpha );
				}

				// Diffuse Texture.
				if(pMesh->GetLayer(0) && pMesh->GetLayer(0)->GetUVs())
				{
					lMappingMode = pMesh->GetLayer(0)->GetUVs()->GetMappingMode();

					KFbxTexture* lCurrentTexture           = NULL;
					KFbxLayerElementTexture* lTextureLayer = NULL;


					if(lDiffuseProperty.IsValid())
					{   
						lCurrentTexture = KFbxCast <KFbxTexture>(lDiffuseProperty.GetSrcObject(KFbxTexture::ClassId, 0)); 
						if( lCurrentTexture )
						{
							Material.Name_ = lCurrentTexture->GetFileName();
						}
					}
				}
			}

			// Grab specular layer.
			{
				lSpecularProperty = lSurfaceMaterial->FindProperty(KFbxSurfaceMaterial::sSpecular);

				// NOTE: Blender doesnt export power :(
				lSpecularPowerProperty = lSurfaceMaterial->FindProperty(KFbxSurfaceMaterial::sSpecularFactor);

				// Specular Colour.
				if(lSpecularProperty.IsValid())
				{   
					KFbxColor Color;
					lSpecularProperty.Get( &Color, eDOUBLE4 );
					Material.SpecularColour_.set( Color.mRed, Color.mGreen, Color.mBlue, Color.mAlpha );
				}

				// Shininess
				if(lSpecularPowerProperty.IsValid())
				{   
					double Power;
					lSpecularPowerProperty.Get( &Power, eDOUBLE1 );
					Material.SpecularPower_ = Power * 128.0f;

					// Cancel out.
					if( Power < 0.01f )
					{
						Material.SpecularColour_.set( 0.0f, 0.0f, 0.0f, 1.0f );
					}
				}

				// Specular Texture.
				/*
				if(pMesh->GetLayer(0) && pMesh->GetLayer(0)->GetUVs())
				{
				lMappingMode = pMesh->GetLayer(0)->GetUVs()->GetMappingMode();

				KFbxTexture* lCurrentTexture           = NULL;
				KFbxLayerElementTexture* lTextureLayer = NULL;

				if(lSpecularProperty.IsValid())
				{   
				lCurrentTexture = KFbxCast <KFbxTexture>(lSpecularProperty.GetSrcObject(KFbxTexture::ClassId, 0)); 
				if( lCurrentTexture )
				{
				Material.Name_ = lCurrentTexture->GetFileName();
				}
				}
				}
				*/
			}
		}
		else
		{
			BcAssert( iMaterial > 0 );
		}

		Material.Name_ = PaStringStripExtension( Material.Name_ );
		pExportMesh->addMaterial( Material );
	}

	// Extract mesh.
	int lPolygonIndex;
	int lPolygonCount = pMesh->GetPolygonCount();

	KFbxLayerElementArrayTemplate<KFbxVector2>* lUVArray = NULL;    
	pMesh->GetTextureUV(&lUVArray, KFbxLayerElement::eDIFFUSE_TEXTURES); 

	KFbxLayerElementArrayTemplate<int>* lMaterialIndices = NULL;    
	pMesh->GetMaterialIndices( &lMaterialIndices );

	int MatIndexCount = lMaterialIndices->GetCount();
	int CurrentMaterialIndex = BcErrorCode;

	KFbxVector4* pVertexPos = pMesh->GetControlPoints();

	for (lPolygonIndex = 0; lPolygonIndex < lPolygonCount; lPolygonIndex++)
	{
		int lCurrentUVIndex;
		int lVerticeCount = pMesh->GetPolygonSize(lPolygonIndex);

		// Grab indices and insert into mesh.
		BcU32 Indices[32];
		MdlIndex ExportedIndices[32];
		BcAssert( lVerticeCount < sizeof( Indices ) / sizeof( Indices[ 0 ] ) );

		for( BcU32 iVert = 0; iVert < lVerticeCount; ++iVert )
		{
			KFbxVector4 FBXNormal;

			// Indices.
			Indices[ iVert ] = pMesh->GetPolygonVertex( lPolygonIndex, iVert );		

			// Normal.
			pMesh->GetPolygonVertexNormal( lPolygonIndex, iVert, FBXNormal );

			// Vertex export.
			MdlVertex ExportVertex;
			KFbxVector4 FBXVertex = pVertexPos[ Indices[ iVert ] ];
	
			lCurrentUVIndex = pMesh->GetTextureUVIndex(lPolygonIndex, iVert);
			//lCurrentNormalIndex = lNormalIndexArray

			if( lUVArray )
			{
				double* pUV = lUVArray->GetAt(lCurrentUVIndex).mData;
				ExportVertex.UV_.x( pUV[0] );
				ExportVertex.UV_.y( pUV[1] );
				ExportVertex.bUV_ = BcTrue;
			}
	
			//
			ExportVertex.Position_.set( FBXVertex[0], FBXVertex[1], FBXVertex[2] );
			ExportVertex.Normal_.set( FBXNormal[0], FBXNormal[1], FBXNormal[2] );
			ExportVertex.bNormal_ = BcTrue;

			// TODO: Share.
			if( lPolygonIndex < MatIndexCount )
			{
				CurrentMaterialIndex = lMaterialIndices->GetAt( lPolygonIndex );
			}
			ExportedIndices[ iVert ].iVertex_ = pExportMesh->addVertex( ExportVertex );
			ExportedIndices[ iVert ].iMaterial_ = CurrentMaterialIndex;
			BcAssert( CurrentMaterialIndex != BcErrorCode );
		}

		// Triangulate indices for this polygon.
		
		for( BcU32 iExportVert = 2; iExportVert < lVerticeCount; ++iExportVert )
		{
			pExportMesh->addIndex( ExportedIndices[ iExportVert - 1 ] );
			pExportMesh->addIndex( ExportedIndices[ iExportVert ] );
			pExportMesh->addIndex( ExportedIndices[ 0 ] );
		}
	}

	//
	pExportMesh->buildTangents();

	// Find AABB.
	pExportNode->aabb( pExportMesh->findAABB() );
}

//////////////////////////////////////////////////////////////////////////
// processLight
void FBXLoader::processLight( KFbxNode* pFBXNode, MdlNode* pExportNode )
{
	KFbxLight* pLight = (KFbxLight*) pFBXNode->GetNodeAttribute();

	pExportNode->type( eNT_LIGHT );
	MdlLight* pLightObj = pExportNode->pLightObject();

	double* pColour = pLight->Color.Get();
	pLightObj->Colour_.set( pColour[0], pColour[1], pColour[2], 1.0f );

	// The cone angle is only relevant if the light is a spot.
	switch( pLight->LightType.Get() )
	{
	case KFbxLight::ePOINT:
		pLightObj->Type_ = MdlLight::T_OMNI;
		break;

	case KFbxLight::eDIRECTIONAL:
		pLightObj->Type_ = MdlLight::T_DIRECTIONAL;
		break;

	case KFbxLight::eSPOT:
		pLightObj->Type_ = MdlLight::T_SPOT;
		break;

	}

	switch( pLight->DecayType.Get() )
	{
	case KFbxLight::eNONE:
		pLightObj->AttnC_ = 0.0f;
		pLightObj->AttnL_ = 1.0f;
		pLightObj->AttnQ_ = 0.0f;
		break;

	case KFbxLight::eLINEAR:
		pLightObj->AttnC_ = 0.0f;
		pLightObj->AttnL_ = 1.0f;
		pLightObj->AttnQ_ = 0.0f;
		break;

	case KFbxLight::eQUADRATIC:
	case KFbxLight::eCUBIC:
		pLightObj->AttnC_ = 0.0f;
		pLightObj->AttnL_ = 1.0f;
		pLightObj->AttnQ_ = 0.1f;
		break;
	}

	// bleh.
	if( pLightObj->Type_ == MdlLight::T_DIRECTIONAL )
	{
		pLightObj->AttnL_ = 0.0f;
		pLightObj->AttnQ_ = 0.0f;
	}
}
