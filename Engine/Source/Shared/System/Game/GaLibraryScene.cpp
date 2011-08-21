/**************************************************************************
*
* File:		GaLibraryScene.cpp
* Author:	Neil Richardson 
* Ver/Date:	21/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaLibraryScene.h"
#include "GaLibraryMath.h"

#include "GaCore.h"

//////////////////////////////////////////////////////////////////////////
// GaSceneCanvas
gmFunctionEntry GaSceneCanvas::GM_TYPELIB[] = 
{
	{ "SetMaterialInstance",		GaSceneCanvas::SetMaterialInstance },
	{ "PushMatrix",					GaSceneCanvas::PushMatrix },
	{ "PopMatrix",					GaSceneCanvas::PopMatrix },
	{ "AddPrimitive",				GaSceneCanvas::AddPrimitive },
	{ "DrawLine",					GaSceneCanvas::DrawLine },
	{ "DrawSprite",					GaSceneCanvas::DrawSprite },
	{ "DrawSpriteCentered",			GaSceneCanvas::DrawSpriteCentered },
	{ "DrawSpriteCentered3D",		GaSceneCanvas::DrawSpriteCentered3D },
	{ "Clear",						GaSceneCanvas::Clear },
	{ "Render",						GaSceneCanvas::Render },
};

int GM_CDECL GaSceneCanvas::Create( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 3 );
	GM_CHECK_STRING_PARAM( pName, 0 )
	GM_CHECK_INT_PARAM( NoofVertices, 1 );
	GM_CHECK_USER_PARAM( ScnMaterialInstance*, GaSceneMaterialInstance::GM_TYPE, pMaterialInstance, 2 );
	
	ScnCanvasRef CanvasRef;
	if( CsCore::pImpl()->createResource( pName, CanvasRef, NoofVertices, pMaterialInstance ) )
	{
		// Allocate a new object instance for the font instance.
		gmUserObject* pUserObj = GaSceneCanvas::AllocUserObject( a_thread->GetMachine(), CanvasRef );
		
		// Push onto stack.
		a_thread->PushUser( pUserObj );
		
		// Add resource block incase it isn't ready.
		return GaCore::pImpl()->addResourceBlock( CanvasRef, pUserObj, a_thread ) ? GM_SYS_BLOCK : GM_OK;
	}
	
	return GM_OK;
}

int GM_CDECL GaSceneCanvas::SetMaterialInstance( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( ScnMaterialInstance*, GaSceneMaterialInstance::GM_TYPE, pMaterialInstance, 0 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->setMaterialInstance( pMaterialInstance );
	
	return GM_OK;
}

int GM_CDECL GaSceneCanvas::PushMatrix( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( BcMat4d*, GaMat4::GM_TYPE, pMatrix, 0 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->pushMatrix( *pMatrix );
	
	return GM_OK;
}

int GM_CDECL GaSceneCanvas::PopMatrix( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->popMatrix();
	
	return GM_OK;
}

int GM_CDECL GaSceneCanvas::AddPrimitive( gmThread* a_thread )
{
	// ( Type, List<Vertex>, Layer, UseMatrixStack )
	// Vertex = List(Pos(vec3), UV(vec2), RGBA(vec4))
	GM_CHECK_NUM_PARAMS( 4 );
	GM_CHECK_INT_PARAM( Type, 0 );
	GM_CHECK_USER_PARAM( gmUserList*, GM_LIST, pList, 1 );
	GM_CHECK_INT_PARAM( Layer, 2 );
	GM_CHECK_INT_PARAM( UseMatrixStack, 3 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	// Allocate enough vertices on the canvas to copy the script vertices into.
	ScnCanvasVertex* pVertices = pCanvas->allocVertices( pList->Length() );
	
	// Copy vertices from list in.
	for( BcU32 Idx = 0; Idx < pList->Length(); ++Idx )
	{
		const gmVariable& Vertex = pList->GetAt( Idx );
		
		gmUserList* pVertexList = (gmUserList*)Vertex.GetUserSafe( GM_LIST );
		if( pVertexList )
		{
			gmVariable Pos = pVertexList->GetAt( 0 );
			gmVariable Nrm = pVertexList->GetAt( 1 );
			gmVariable Tan = pVertexList->GetAt( 2 );
			gmVariable UV = pVertexList->GetAt( 3 );
			gmVariable RGBA = pVertexList->GetAt( 4 );
			
			BcVec3d* pPos = (BcVec3d*)Pos.GetUserSafe( GaVec3::GM_TYPE );
			BcVec3d* pNrm = (BcVec3d*)Nrm.GetUserSafe( GaVec3::GM_TYPE );
			BcVec3d* pTan = (BcVec3d*)Tan.GetUserSafe( GaVec3::GM_TYPE );
			BcVec2d* pUV = (BcVec2d*)UV.GetUserSafe( GaVec2::GM_TYPE );
			BcVec4d* pRGBA = (BcVec4d*)RGBA.GetUserSafe( GaVec4::GM_TYPE );
			
			if( pPos == NULL || pUV == NULL || pRGBA == NULL )
			{
				return GM_EXCEPTION;
			}
			
			// Copy into vertex.
			ScnCanvasVertex* pOutputVertex = &pVertices[ Idx ];
			
			pOutputVertex->X_ = pPos->x();
			pOutputVertex->Y_ = pPos->y();
			pOutputVertex->Z_ = pPos->z();

			pOutputVertex->NX_ = pNrm->x();
			pOutputVertex->NY_ = pNrm->y();
			pOutputVertex->NZ_ = pNrm->z();

			pOutputVertex->TX_ = pTan->x();
			pOutputVertex->TY_ = pTan->y();
			pOutputVertex->TZ_ = pTan->z();

			pOutputVertex->U_ = pUV->x();
			pOutputVertex->V_ = pUV->y();
			
			pOutputVertex->RGBA_ = RsColour( *pRGBA ).asABGR();
		}
		else
		{
			return GM_EXCEPTION;
		}
	}
		
	// Add primitive to canvas.
	pCanvas->addPrimitive( (eRsPrimitiveType)Type, pVertices, pList->Length(), Layer, UseMatrixStack ? BcTrue : BcFalse );
	
	//
	return GM_OK;
}

int GM_CDECL GaSceneCanvas::DrawLine( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 4 );
	GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pA, 0 );
	GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pB, 1 );
	GM_CHECK_USER_PARAM( BcVec4d*, GaVec4::GM_TYPE, pColour, 2 );
	GM_CHECK_INT_PARAM( Layer, 3 );

	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->drawLine( *pA, *pB, *pColour, Layer );
	
	return GM_OK;
};

int GM_CDECL GaSceneCanvas::DrawSprite( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 5 );
	GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pPosition, 0 );
	GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pSize, 1 );
	GM_CHECK_INT_PARAM( TextureIdx, 2 );
	GM_CHECK_USER_PARAM( BcVec4d*, GaVec4::GM_TYPE, pColour, 3 );
	GM_CHECK_INT_PARAM( Layer, 4 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();

	pCanvas->drawSprite( *pPosition, *pSize, TextureIdx, *pColour, Layer );
	
	return GM_OK;
}

int GM_CDECL GaSceneCanvas::DrawSpriteCentered( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 5 );
	GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pPosition, 0 );
	GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pSize, 1 );
	GM_CHECK_INT_PARAM( TextureIdx, 2 );
	GM_CHECK_USER_PARAM( BcVec4d*, GaVec4::GM_TYPE, pColour, 3 );
	GM_CHECK_INT_PARAM( Layer, 4 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->drawSpriteCentered( *pPosition, *pSize, TextureIdx, *pColour, Layer );
	
	return GM_OK;
}

int GM_CDECL GaSceneCanvas::DrawSpriteCentered3D( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 5 );
	GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pPosition, 0 );
	GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pSize, 1 );
	GM_CHECK_INT_PARAM( TextureIdx, 2 );
	GM_CHECK_USER_PARAM( BcVec4d*, GaVec4::GM_TYPE, pColour, 3 );
	GM_CHECK_INT_PARAM( Layer, 4 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->drawSpriteCentered3D( *pPosition, *pSize, TextureIdx, *pColour, Layer );
	
	return GM_OK;
}

int GM_CDECL GaSceneCanvas::Clear( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->clear();
	
	return GM_OK;
}

int GM_CDECL GaSceneCanvas::Render( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( RsFrame*, GaSceneFrame::GM_TYPE, pFrame, 0 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->render( pFrame, RsRenderSort( 0 ) );
	
	return GM_OK;
}

void GM_CDECL GaSceneCanvas::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnCanvas >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaSceneCanvas::GM_TYPELIB ) / sizeof( GaSceneCanvas::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaSceneCanvas::GM_TYPE, GaSceneCanvas::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaSceneFont
gmFunctionEntry GaSceneFont::GM_TYPELIB[] = 
{
	{ "CreateInstance",			GaSceneFont::CreateInstance }
};

int GM_CDECL GaSceneFont::CreateInstance( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 2 );
	GM_CHECK_STRING_PARAM( pName, 0 );
	GM_CHECK_USER_PARAM( ScnMaterial*, GaSceneMaterial::GM_TYPE, pMaterial, 1 );

	ScnFont* pFont = (ScnFont*)a_thread->ThisUser_NoChecks();
	
	// Attempt to create instance.
	ScnFontInstanceRef FontInstanceRef;
	if( pFont->createInstance( pName, FontInstanceRef, pMaterial ) )
	{
		// Allocate a new object instance for the font instance.
		gmUserObject* pUserObj = GaSceneFontInstance::AllocUserObject( a_thread->GetMachine(), FontInstanceRef );
		
		// Push onto stack.
		a_thread->PushUser( pUserObj );

		// Add resource block incase it isn't ready.
		return GaCore::pImpl()->addResourceBlock( FontInstanceRef, pUserObj, a_thread ) ? GM_SYS_BLOCK : GM_OK;
	}
	
	return GM_OK;
}

void GM_CDECL GaSceneFont::CreateType( gmMachine* a_machine )
{
	// Create base type.
	GaLibraryResource< ScnFont >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaSceneFont::GM_TYPELIB ) / sizeof( GaSceneFont::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaSceneFont::GM_TYPE, GaSceneFont::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaSceneFontInstance
gmFunctionEntry GaSceneFontInstance::GM_TYPELIB[] = 
{
	{ "Draw",					GaSceneFontInstance::Draw },
	{ "GetMaterialInstance",	GaSceneFontInstance::GetMaterialInstance }
};

int GM_CDECL GaSceneFontInstance::Draw( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 2 );
	GM_CHECK_USER_PARAM( ScnCanvas*, GaSceneCanvas::GM_TYPE, pCanvas, 0 );
	GM_CHECK_STRING_PARAM( pText, 1 );
	ScnFontInstance* pFontInstance = (ScnFontInstance*)a_thread->ThisUser_NoChecks();

	pFontInstance->draw( pCanvas, pText );
	
	return GM_OK;
}

int GM_CDECL GaSceneFontInstance::GetMaterialInstance( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	ScnFontInstance* pFontInstance = (ScnFontInstance*)a_thread->ThisUser_NoChecks();

	ScnMaterialInstanceRef MaterialInstanceRef = pFontInstance->getMaterialInstance();

	a_thread->PushUser( GaSceneMaterialInstance::AllocUserObject( a_thread->GetMachine(), MaterialInstanceRef ) );
	
	return GM_OK;
}

void GM_CDECL GaSceneFontInstance::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnFontInstance >::CreateType( a_machine );

	// Register type library.
	int NoofEntries = sizeof( GaSceneFontInstance::GM_TYPELIB ) / sizeof( GaSceneFontInstance::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaSceneFontInstance::GM_TYPE, GaSceneFontInstance::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaSceneMaterial
gmFunctionEntry GaSceneMaterial::GM_TYPELIB[] = 
{
	{ "CreateInstance",			GaSceneMaterial::CreateInstance }
};

int GM_CDECL GaSceneMaterial::CreateInstance( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_STRING_PARAM( pName, 0 );
	
	ScnMaterial* pMaterial = (ScnMaterial*)a_thread->ThisUser_NoChecks();
	
	// Attempt to create instance.
	ScnMaterialInstanceRef MaterialInstanceRef;
	if( pMaterial->createInstance( pName, MaterialInstanceRef, scnSPF_DEFAULT ) )
	{
		// Allocate a new object instance for the font instance.
		gmUserObject* pUserObj = GaSceneMaterialInstance::AllocUserObject( a_thread->GetMachine(), MaterialInstanceRef );
		
		// Push onto stack.
		a_thread->PushUser( pUserObj );
		
		// Add resource block incase it isn't ready.
		return GaCore::pImpl()->addResourceBlock( MaterialInstanceRef, pUserObj, a_thread ) ? GM_SYS_BLOCK : GM_OK;
	}
	
	return GM_OK;
}

void GM_CDECL GaSceneMaterial::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnMaterial >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaSceneMaterial::GM_TYPELIB ) / sizeof( GaSceneMaterial::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaSceneMaterial::GM_TYPE, GaSceneMaterial::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaSceneMaterialInstance
gmFunctionEntry GaSceneMaterialInstance::GM_TYPELIB[] = 
{
	{ "FindParameter",			GaSceneMaterialInstance::FindParameter },
	{ "SetParameter",			GaSceneMaterialInstance::SetParameter },
	{ "SetTexture",				GaSceneMaterialInstance::SetTexture },
	{ "GetTexture",				GaSceneMaterialInstance::GetTexture }
};

int GM_CDECL GaSceneMaterialInstance::FindParameter( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_STRING_PARAM( pName, 0 );
	
	ScnMaterialInstance* pMaterialInstance = (ScnMaterialInstance*)a_thread->ThisUser_NoChecks();
	
	BcU32 ParameterID = pMaterialInstance->findParameter( pName );
	
	if( ParameterID != BcErrorCode )
	{
		a_thread->PushInt( ParameterID );
	}
	else
	{
		a_thread->PushInt( BcErrorCode );
	}
	
	return GM_OK;
}

int GM_CDECL GaSceneMaterialInstance::SetParameter( gmThread* a_thread )
{
	if( a_thread->GetNumParams() == 2 )
	{
		// Set individual.
		GM_CHECK_INT_PARAM( ParameterID, 0 );
		ScnMaterialInstance* pMaterialInstance = (ScnMaterialInstance*)a_thread->ThisUser_NoChecks();

		gmType ParamType = a_thread->ParamType( 1 );
		
		// TODO: Factor this into a seperate function.
		if( ParamType == GM_INT )
		{
			GM_CHECK_INT_PARAM( Value, 1 );
			if( Value == 0 || Value == 1 )
			{
				pMaterialInstance->setParameter( (BcU32)ParameterID, Value ? BcTrue : BcFalse );
			}
			else
			{
				pMaterialInstance->setParameter( (BcU32)ParameterID, Value );
			}
		}
		else if ( ParamType == GM_FLOAT )
		{
			GM_CHECK_FLOAT_PARAM( Value, 1 );
			pMaterialInstance->setParameter( (BcU32)ParameterID, Value );
		}
		else if ( ParamType == GaVec2::GM_TYPE )
		{
			GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pValue, 1 );
			pMaterialInstance->setParameter( (BcU32)ParameterID, *pValue );
		}
		else if ( ParamType == GaVec3::GM_TYPE )
		{
			GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pValue, 1 );
			pMaterialInstance->setParameter( (BcU32)ParameterID, *pValue );
		}
		else if ( ParamType == GaVec4::GM_TYPE )
		{
			GM_CHECK_USER_PARAM( BcVec4d*, GaVec4::GM_TYPE, pValue, 1 );
			pMaterialInstance->setParameter( (BcU32)ParameterID, *pValue );
		}
		else if ( ParamType == GaMat4::GM_TYPE )
		{
			GM_CHECK_USER_PARAM( BcMat4d*, GaMat4::GM_TYPE, pValue, 1 );
			pMaterialInstance->setParameter( (BcU32)ParameterID, *pValue );
		}
	}
	else if( a_thread->GetNumParams() == 1 )
	{
		// Set from table.
		GM_CHECK_TABLE_PARAM( pTable, 0 );
		BcUnusedVar( pTable );
	
		// Not implemented yet!
		return GM_EXCEPTION;		
	}
	
	return GM_OK;
}

int GM_CDECL GaSceneMaterialInstance::SetTexture( gmThread* a_thread )
{
	if( a_thread->GetNumParams() == 2 )
	{
		// Set individual.
		GM_CHECK_INT_PARAM( ParameterID, 0 );
		GM_CHECK_USER_PARAM( ScnTexture*, GaSceneTexture::GM_TYPE, pTexture, 1 );
		ScnMaterialInstance* pMaterialInstance = (ScnMaterialInstance*)a_thread->ThisUser_NoChecks();
		
		pMaterialInstance->setTexture( (BcU32)ParameterID, pTexture );
	}
	
	return GM_OK;
}

int GM_CDECL GaSceneMaterialInstance::GetTexture( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_INT_PARAM( Idx, 0 );
	
	ScnMaterialInstance* pMaterialInstance = (ScnMaterialInstance*)a_thread->ThisUser_NoChecks();
	
	ScnTextureRef Texture = pMaterialInstance->getTexture( Idx );
	
	if( Texture.isValid() )
	{
		a_thread->PushUser( GaSceneTexture::AllocUserObject( a_thread->GetMachine(), Texture ) );
	}
	
	return GM_OK;
}

void GM_CDECL GaSceneMaterialInstance::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnMaterialInstance >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaSceneMaterialInstance::GM_TYPELIB ) / sizeof( GaSceneMaterialInstance::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaSceneMaterialInstance::GM_TYPE, GaSceneMaterialInstance::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaSceneTexture
gmFunctionEntry GaSceneTexture::GM_TYPELIB[] = 
{
	{ "GetWidth",				GaSceneTexture::GetWidth },
	{ "GetHeight",				GaSceneTexture::GetHeight },
	{ "GetTexel",				GaSceneTexture::GetTexel }
};

int GM_CDECL GaSceneTexture::GetWidth( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS(0);
	ScnTexture* pTexture = (ScnTexture*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushInt( (int)pTexture->getWidth() );
	
	return GM_OK;
}

int GM_CDECL GaSceneTexture::GetHeight( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS(0);
	ScnTexture* pTexture = (ScnTexture*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushInt( (int)pTexture->getHeight() );
	
	return GM_OK;
}

int GM_CDECL GaSceneTexture::GetTexel( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS(2);
	GM_CHECK_INT_PARAM( X, 0 );
	GM_CHECK_INT_PARAM( Y, 1 );
	ScnTexture* pTexture = (ScnTexture*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushNewUser( GaVec4::Alloc( a_thread->GetMachine(), pTexture->getTexel( X, Y ) ), GaVec4::GM_TYPE );

	return GM_OK;
}

void GM_CDECL GaSceneTexture::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnTexture >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaSceneTexture::GM_TYPELIB ) / sizeof( GaSceneTexture::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaSceneTexture::GM_TYPE, GaSceneTexture::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaSceneRenderTarget
gmFunctionEntry GaSceneRenderTarget::GM_TYPELIB[] = 
{
	{ "Bind",					GaSceneRenderTarget::Bind },
	{ "Unbind",					GaSceneRenderTarget::Unbind }
};

int GM_CDECL GaSceneRenderTarget::Create( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 3 );
	GM_CHECK_STRING_PARAM( pName, 0 )
	GM_CHECK_INT_PARAM( Width, 1 )
	GM_CHECK_INT_PARAM( Height, 2 );
	
	ScnRenderTargetRef RenderTargetRef;
	if( CsCore::pImpl()->createResource( pName, RenderTargetRef, Width, Height ) )
	{
		// Allocate a new object instance for the font instance.
		gmUserObject* pUserObj = GaSceneRenderTarget::AllocUserObject( a_thread->GetMachine(), RenderTargetRef );
		
		// Push onto stack.
		a_thread->PushUser( pUserObj );
		
		// Add resource block incase it isn't ready.
		return GaCore::pImpl()->addResourceBlock( RenderTargetRef, pUserObj, a_thread ) ? GM_SYS_BLOCK : GM_OK;
	}
	
	return GM_OK;
}

int GM_CDECL GaSceneRenderTarget::Bind( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( RsFrame*, GaSceneFrame::GM_TYPE, pFrame, 0 );
	ScnRenderTarget* pRenderTarget = (ScnRenderTarget*)a_thread->ThisUser_NoChecks();
	
	pRenderTarget->bind( pFrame );
	
	return GM_OK;
}

int GM_CDECL GaSceneRenderTarget::Unbind( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( RsFrame*, GaSceneFrame::GM_TYPE, pFrame, 0 );
	ScnRenderTarget* pRenderTarget = (ScnRenderTarget*)a_thread->ThisUser_NoChecks();
	
	pRenderTarget->unbind( pFrame );
	
	return GM_OK;
}

void GM_CDECL GaSceneRenderTarget::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnRenderTarget >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaSceneRenderTarget::GM_TYPELIB ) / sizeof( GaSceneRenderTarget::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaSceneRenderTarget::GM_TYPE, GaSceneRenderTarget::GM_TYPELIB, NoofEntries );
}


//////////////////////////////////////////////////////////////////////////
// GaSceneSound
gmFunctionEntry GaSceneSound::GM_TYPELIB[] = 
{
	{ NULL, NULL }
};

void GM_CDECL GaSceneSound::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnSound >::CreateType( a_machine );
	
	// Register type library.
	//int NoofEntries = sizeof( GaSceneSound::GM_TYPELIB ) / sizeof( GaSceneSound::GM_TYPELIB[0] );
	//a_machine->RegisterTypeLibrary( GaSceneSound::GM_TYPE, GaSceneSound::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaSceneSoundEmitter
gmFunctionEntry GaSceneSoundEmitter::GM_TYPELIB[] = 
{
	{ "Play",				GaSceneSoundEmitter::Play }
};

int GM_CDECL GaSceneSoundEmitter::Create( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_STRING_PARAM( pName, 0 )

	ScnSoundEmitterRef SoundEmitterRef;
	if( CsCore::pImpl()->createResource( pName, SoundEmitterRef ) )
	{
		// Allocate a new object instance for the font instance.
		gmUserObject* pUserObj = GaSceneSoundEmitter::AllocUserObject( a_thread->GetMachine(), SoundEmitterRef );
		
		// Push onto stack.
		a_thread->PushUser( pUserObj );
		
		// Add resource block incase it isn't ready.
		return GaCore::pImpl()->addResourceBlock( SoundEmitterRef, pUserObj, a_thread ) ? GM_SYS_BLOCK : GM_OK;
	}
	
	return GM_OK;
}

int GM_CDECL GaSceneSoundEmitter::Play( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( ScnSound*, GaSceneSound::GM_TYPE, pSound, 0 );
	ScnSoundEmitter* pSoundEmitter = (ScnSoundEmitter*)a_thread->ThisUser_NoChecks();

	pSoundEmitter->play( pSound );
		
	return GM_OK;
}

void GM_CDECL GaSceneSoundEmitter::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnSoundEmitter >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaSceneSoundEmitter::GM_TYPELIB ) / sizeof( GaSceneSoundEmitter::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaSceneSoundEmitter::GM_TYPE, GaSceneSoundEmitter::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaSceneSynthesizer
gmFunctionEntry GaSceneSynthesizer::GM_TYPELIB[] = 
{
	{ "SetCommand",				GaSceneSynthesizer::SetCommand },
	{ "NextPattern",			GaSceneSynthesizer::NextPattern },
	{ "AddSynthModule",			GaSceneSynthesizer::AddSynthModule },
	{ "HadBeatEvent",			GaSceneSynthesizer::HadBeatEvent },
	{ "HadQueuePatternEvent",	GaSceneSynthesizer::HadQueuePatternEvent },
	{ "HadLastPatternEvent",	GaSceneSynthesizer::HadLastPatternEvent },
	{ "GetCommandIndex",		GaSceneSynthesizer::GetCommandIndex },
	{ "GetFreq",				GaSceneSynthesizer::GetFreq },
	{ "PlayEffect",				GaSceneSynthesizer::PlayEffect }
};

int GM_CDECL GaSceneSynthesizer::Create( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 2 );
	GM_CHECK_STRING_PARAM( pName, 0 )
	GM_CHECK_FLOAT_PARAM( SampleRate, 1 )
	
	ScnSynthesizerRef SynthesizerRef;
	if( CsCore::pImpl()->createResource( pName, SynthesizerRef, SampleRate ) )
	{
		// Allocate a new object instance for the font instance.
		gmUserObject* pUserObj = GaSceneSynthesizer::AllocUserObject( a_thread->GetMachine(), SynthesizerRef );
		
		// Push onto stack.
		a_thread->PushUser( pUserObj );
		
		// Add resource block incase it isn't ready.
		return GaCore::pImpl()->addResourceBlock( SynthesizerRef, pUserObj, a_thread ) ? GM_SYS_BLOCK : GM_OK;
	}
	
	return GM_OK;
}

int GM_CDECL GaSceneSynthesizer::SetCommand( gmThread* a_thread )
{
	GM_CHECK_INT_PARAM( TrackIdx, 0 );
	GM_CHECK_INT_PARAM( CommandIdx, 1 );
	GM_CHECK_INT_PARAM( Type, 2 );
	float ValueA = 0.0f, ValueB = 0.0f, ValueC = 0.0f, ValueD = 0.0f;
	gmGetFloatOrIntParamAsFloat( a_thread, 3, ValueA );
	gmGetFloatOrIntParamAsFloat( a_thread, 4, ValueB );
	gmGetFloatOrIntParamAsFloat( a_thread, 5, ValueC );
	gmGetFloatOrIntParamAsFloat( a_thread, 6, ValueD );

	ScnSynthesizer* pSynthesizer = (ScnSynthesizer*)a_thread->ThisUser_NoChecks();
	
	pSynthesizer->setCommand( TrackIdx, CommandIdx, (ScnSynthesizer::TCommandType)Type, ValueA, ValueB, ValueC, ValueD );
	
	return GM_OK;
}

int GM_CDECL GaSceneSynthesizer::NextPattern( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );

	ScnSynthesizer* pSynthesizer = (ScnSynthesizer*)a_thread->ThisUser_NoChecks();
	
	pSynthesizer->nextPattern();
	
	return GM_OK;	
}

int GM_CDECL GaSceneSynthesizer::AddSynthModule( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_INT_PARAM( Type, 0 );
	
	ScnSynthesizer* pSynthesizer = (ScnSynthesizer*)a_thread->ThisUser_NoChecks();
	
	BcU32 ID = pSynthesizer->addSynthModule( (ScnSynthesizer::TSynthModuleType)Type );
	
	a_thread->PushInt( ID );
	
	return GM_OK;
}

int GM_CDECL GaSceneSynthesizer::HadBeatEvent( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	
	ScnSynthesizer* pSynthesizer = (ScnSynthesizer*)a_thread->ThisUser_NoChecks();
	
	BcBool RetVal = pSynthesizer->hadBeatEvent();
	
	a_thread->PushInt( RetVal );
	
	return GM_OK;
}

int GM_CDECL GaSceneSynthesizer::HadQueuePatternEvent( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	
	ScnSynthesizer* pSynthesizer = (ScnSynthesizer*)a_thread->ThisUser_NoChecks();
	
	BcBool RetVal = pSynthesizer->hadQueuePatternEvent();
	
	a_thread->PushInt( RetVal );
	
	return GM_OK;
}

int GM_CDECL GaSceneSynthesizer::HadLastPatternEvent( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	
	ScnSynthesizer* pSynthesizer = (ScnSynthesizer*)a_thread->ThisUser_NoChecks();
	
	BcBool RetVal = pSynthesizer->hadLastPatternEvent();
	
	a_thread->PushInt( RetVal );
	
	return GM_OK;
}

int GM_CDECL GaSceneSynthesizer::GetCommandIndex( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	
	ScnSynthesizer* pSynthesizer = (ScnSynthesizer*)a_thread->ThisUser_NoChecks();
	
	BcU32 RetVal = pSynthesizer->getCommandIndex();
	
	a_thread->PushInt( RetVal );
	
	return GM_OK;
}

int GM_CDECL GaSceneSynthesizer::GetFreq( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_FLOAT_OR_INT_PARAM( MIDINote, 0 );
	
	ScnSynthesizer* pSynthesizer = (ScnSynthesizer*)a_thread->ThisUser_NoChecks();
	
	BcReal RetVal = pSynthesizer->getFreq( (BcU32)MIDINote );
	
	a_thread->PushFloat( RetVal );
	
	return GM_OK;
}

int GM_CDECL GaSceneSynthesizer::PlayEffect( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 3 );	
	GM_CHECK_INT_PARAM( TrackIdx, 0 );
	GM_CHECK_FLOAT_PARAM( StartFreq, 1 );
	GM_CHECK_FLOAT_PARAM( EndFreq, 2 );
	
	ScnSynthesizer* pSynthesizer = (ScnSynthesizer*)a_thread->ThisUser_NoChecks();
	
	pSynthesizer->playEffect( TrackIdx, StartFreq, EndFreq );
	
	return GM_OK;
}

void GM_CDECL GaSceneSynthesizer::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnSynthesizer >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaSceneSynthesizer::GM_TYPELIB ) / sizeof( GaSceneSynthesizer::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaSceneSynthesizer::GM_TYPE, GaSceneSynthesizer::GM_TYPELIB, NoofEntries );
}


//////////////////////////////////////////////////////////////////////////
// GaSceneFrame - Very quick and dirty way to access RsFrame for rendering.
gmType GaSceneFrame::GM_TYPE = GM_NULL;

bool GM_CDECL GaSceneFrame::Trace( gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workRemaining, int& a_workDone )
{
	// Do nothing.
	++a_workDone;
	return false;
}

void GM_CDECL GaSceneFrame::Destruct( gmMachine* a_machine, gmUserObject* a_object )
{
	// Do nothing.
}

void GM_CDECL GaSceneFrame::AsString( gmUserObject* a_object, char* a_buffer, int a_bufferLen )
{
	RsFrame* pObj = (RsFrame*)a_object->m_user;

	BcSPrintf( a_buffer, "<RsFrame Object @ %p>", pObj );
}

int GM_CDECL GaSceneFrame::FrameBegin( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	
	RsFrame* pFrame = RsCore::pImpl()->allocateFrame();
	
	// TEMP: Setup default viewport.
	RsViewport Viewport( 0, 0, 1280, 720 );
	//BcReal W = 1280.0f * 0.5f;
	//BcReal H = 720.0f * 0.5f;
	
	pFrame->setRenderTarget( NULL );
	pFrame->setViewport( Viewport );
	
	a_thread->PushNewUser( pFrame, GM_TYPE );
	
	return GM_OK;
}

int GM_CDECL GaSceneFrame::FrameEnd( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( RsFrame*, GaSceneFrame::GM_TYPE, pFrame, 0 );
	
	RsCore::pImpl()->queueFrame( pFrame );
		
	// Yield so renderer picks up the frame next tick.
	return GM_SYS_YIELD;
}

void GM_CDECL GaSceneFrame::CreateType( gmMachine* a_machine )
{
	GM_TYPE = a_machine->CreateUserType( "Frame" );
	
	a_machine->RegisterUserCallbacks( GaSceneFrame::GM_TYPE,
									 &GaSceneFrame::Trace,
									 &GaSceneFrame::Destruct,
									 &GaSceneFrame::AsString ); 
}

//////////////////////////////////////////////////////////////////////////
// gLibScene
static gmFunctionEntry gLibScene[] = 
{
	// Resource requesting.
	{ "Font",					GaSceneFont::Request },
	{ "Material",				GaSceneMaterial::Request },
	{ "Model",					GaSceneModel::Request },
	{ "Shader",					GaSceneShader::Request },
	{ "Texture",				GaSceneTexture::Request },
	{ "Sound",					GaSceneSound::Request },
	
	// Resource creating.
	{ "Canvas",					GaSceneCanvas::Create },
	{ "SoundEmitter",			GaSceneSoundEmitter::Create },
	{ "RenderTarget",			GaSceneRenderTarget::Create },
	{ "Synthesizer",			GaSceneSynthesizer::Create },
	
	// Hacky RsFrame stuff.
	{ "FrameBegin",				GaSceneFrame::FrameBegin },
	{ "FrameEnd",				GaSceneFrame::FrameEnd },
};


//////////////////////////////////////////////////////////////////////////
// GaLibrarySceneBinder
void GaLibrarySceneBinder( gmMachine* a_machine )
{
	// Register types.
	GaSceneFont::CreateType( a_machine );
	GaSceneFontInstance::CreateType( a_machine );
	GaSceneMaterial::CreateType( a_machine );
	GaSceneMaterialInstance::CreateType( a_machine );
	GaSceneModel::CreateType( a_machine );
	GaSceneModelInstance::CreateType( a_machine );
	GaSceneShader::CreateType( a_machine );
	GaSceneTexture::CreateType( a_machine );
	GaSceneRenderTarget::CreateType( a_machine );
	GaSceneSound::CreateType( a_machine );
	GaSceneSoundEmitter::CreateType( a_machine );
	GaSceneSynthesizer::CreateType( a_machine );

	// 
	GaSceneCanvas::CreateType( a_machine );
	
	//
	GaSceneFrame::CreateType( a_machine );
	
	// Register library.
	a_machine->RegisterLibrary( gLibScene, sizeof( gLibScene ) / sizeof( gLibScene[0] ), "Scene" );
}


