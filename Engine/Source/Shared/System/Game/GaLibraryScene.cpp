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
// GaCanvas
gmFunctionEntry GaCanvas::GM_TYPELIB[] = 
{
	{ "SetMaterialInstance",		GaCanvas::SetMaterialInstance },
	{ "PushMatrix",					GaCanvas::PushMatrix },
	{ "PopMatrix",					GaCanvas::PopMatrix },
	{ "DrawSprite",					GaCanvas::DrawSprite },
	{ "DrawSpriteCentered",			GaCanvas::DrawSpriteCentered },
	{ "Clear",						GaCanvas::Clear },
	{ "Render",						GaCanvas::Render },
};

int GM_CDECL GaCanvas::Create( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 3 );
	GM_CHECK_STRING_PARAM( pName, 0 )
	GM_CHECK_INT_PARAM( NoofVertices, 1 );
	GM_CHECK_USER_PARAM( ScnMaterialInstance*, GaMaterialInstance::GM_TYPE, pMaterialInstance, 2 );
	
	ScnCanvasRef CanvasRef;
	if( CsCore::pImpl()->createResource( pName, CanvasRef, NoofVertices, pMaterialInstance ) )
	{
		// Allocate a new object instance for the font instance.
		gmUserObject* pUserObj = GaCanvas::AllocUserObject( a_thread->GetMachine(), CanvasRef );
		
		// Push onto stack.
		a_thread->PushUser( pUserObj );
		
		// Add resource block incase it isn't ready.
		return GaCore::pImpl()->addResourceBlock( CanvasRef, pUserObj, a_thread ) ? GM_SYS_BLOCK : GM_OK;
	}
	
	return GM_OK;
}

int GM_CDECL GaCanvas::SetMaterialInstance( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( ScnMaterialInstance*, GaMaterialInstance::GM_TYPE, pMaterialInstance, 0 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->setMaterialInstance( pMaterialInstance );
	
	return GM_OK;
}

int GM_CDECL GaCanvas::PushMatrix( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( BcMat4d*, GaMat4::GM_TYPE, pMatrix, 0 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->pushMatrix( *pMatrix );
	
	return GM_OK;
}

int GM_CDECL GaCanvas::PopMatrix( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->popMatrix();
	
	return GM_OK;
}

int GM_CDECL GaCanvas::DrawSprite( gmThread* a_thread )
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

int GM_CDECL GaCanvas::DrawSpriteCentered( gmThread* a_thread )
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

int GM_CDECL GaCanvas::Clear( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->clear();
	
	return GM_OK;
}

int GM_CDECL GaCanvas::Render( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( RsFrame*, GaFrame::GM_TYPE, pFrame, 0 );
	ScnCanvas* pCanvas = (ScnCanvas*)a_thread->ThisUser_NoChecks();
	
	pCanvas->render( pFrame, RsRenderSort( 0 ) );
	
	return GM_OK;
}

void GM_CDECL GaCanvas::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnCanvas >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaCanvas::GM_TYPELIB ) / sizeof( GaCanvas::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaCanvas::GM_TYPE, GaCanvas::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaFont
gmFunctionEntry GaFont::GM_TYPELIB[] = 
{
	{ "CreateInstance",			GaFont::CreateInstance }
};

int GM_CDECL GaFont::CreateInstance( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 2 );
	GM_CHECK_STRING_PARAM( pName, 0 );
	GM_CHECK_USER_PARAM( ScnMaterial*, GaMaterial::GM_TYPE, pMaterial, 1 );

	ScnFont* pFont = (ScnFont*)a_thread->ThisUser_NoChecks();
	
	// Attempt to create instance.
	ScnFontInstanceRef FontInstanceRef;
	if( pFont->createInstance( pName, FontInstanceRef, pMaterial ) )
	{
		// Allocate a new object instance for the font instance.
		gmUserObject* pUserObj = GaFontInstance::AllocUserObject( a_thread->GetMachine(), FontInstanceRef );
		
		// Push onto stack.
		a_thread->PushUser( pUserObj );

		// Add resource block incase it isn't ready.
		return GaCore::pImpl()->addResourceBlock( FontInstanceRef, pUserObj, a_thread ) ? GM_SYS_BLOCK : GM_OK;
	}
	
	return GM_OK;
}

void GM_CDECL GaFont::CreateType( gmMachine* a_machine )
{
	// Create base type.
	GaLibraryResource< ScnFont >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaFont::GM_TYPELIB ) / sizeof( GaFont::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaFont::GM_TYPE, GaFont::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaFontInstance
gmFunctionEntry GaFontInstance::GM_TYPELIB[] = 
{
	{ "Draw",					GaFontInstance::Draw },
	{ "GetMaterialInstance",	GaFontInstance::GetMaterialInstance }
};

int GM_CDECL GaFontInstance::Draw( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 2 );
	GM_CHECK_USER_PARAM( ScnCanvas*, GaCanvas::GM_TYPE, pCanvas, 0 );
	GM_CHECK_STRING_PARAM( pText, 1 );
	ScnFontInstance* pFontInstance = (ScnFontInstance*)a_thread->ThisUser_NoChecks();

	pFontInstance->draw( pCanvas, pText );
	
	return GM_OK;
}

int GM_CDECL GaFontInstance::GetMaterialInstance( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	ScnFontInstance* pFontInstance = (ScnFontInstance*)a_thread->ThisUser_NoChecks();

	ScnMaterialInstanceRef MaterialInstanceRef = pFontInstance->getMaterialInstance();

	a_thread->PushUser( GaMaterialInstance::AllocUserObject( a_thread->GetMachine(), MaterialInstanceRef ) );
	
	return GM_OK;
}

void GM_CDECL GaFontInstance::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnFontInstance >::CreateType( a_machine );

	// Register type library.
	int NoofEntries = sizeof( GaFontInstance::GM_TYPELIB ) / sizeof( GaFontInstance::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaFontInstance::GM_TYPE, GaFontInstance::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaMaterial
gmFunctionEntry GaMaterial::GM_TYPELIB[] = 
{
	{ "CreateInstance",			GaMaterial::CreateInstance }
};

int GM_CDECL GaMaterial::CreateInstance( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_STRING_PARAM( pName, 0 );
	
	ScnMaterial* pMaterial = (ScnMaterial*)a_thread->ThisUser_NoChecks();
	
	// Attempt to create instance.
	ScnMaterialInstanceRef MaterialInstanceRef;
	if( pMaterial->createInstance( pName, MaterialInstanceRef, scnSPF_DEFAULT ) )
	{
		// Allocate a new object instance for the font instance.
		gmUserObject* pUserObj = GaMaterialInstance::AllocUserObject( a_thread->GetMachine(), MaterialInstanceRef );
		
		// Push onto stack.
		a_thread->PushUser( pUserObj );
		
		// Add resource block incase it isn't ready.
		return GaCore::pImpl()->addResourceBlock( MaterialInstanceRef, pUserObj, a_thread ) ? GM_SYS_BLOCK : GM_OK;
	}
	
	return GM_OK;
}

void GM_CDECL GaMaterial::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnMaterial >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaMaterial::GM_TYPELIB ) / sizeof( GaMaterial::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaMaterial::GM_TYPE, GaMaterial::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaMaterialInstance
gmFunctionEntry GaMaterialInstance::GM_TYPELIB[] = 
{
	{ "FindParameter",			GaMaterialInstance::FindParameter },
	{ "SetParameter",			GaMaterialInstance::SetParameter },
	{ "GetTexture",				GaMaterialInstance::GetTexture }
};

int GM_CDECL GaMaterialInstance::FindParameter( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_STRING_PARAM( pName, 0 );
	
	ScnMaterialInstance* pMaterialInstance = (ScnMaterialInstance*)a_thread->ThisUser_NoChecks();
	
	BcU32 ParameterID = pMaterialInstance->findParameter( pName );
	
	if( ParameterID != BcErrorCode )
	{
		a_thread->PushInt( ParameterID );
	}
	
	return GM_OK;
}

int GM_CDECL GaMaterialInstance::SetParameter( gmThread* a_thread )
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
	
		// Not implemented yet!
		return GM_EXCEPTION;		
	}
	
	return GM_OK;
}

int GM_CDECL GaMaterialInstance::GetTexture( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_INT_PARAM( Idx, 0 );
	
	ScnMaterialInstance* pMaterialInstance = (ScnMaterialInstance*)a_thread->ThisUser_NoChecks();
	
	ScnTextureRef Texture = pMaterialInstance->getTexture( Idx );
	
	if( Texture.isValid() )
	{
		a_thread->PushUser( GaTexture::AllocUserObject( a_thread->GetMachine(), Texture ) );
	}
	
	return GM_OK;
}

void GM_CDECL GaMaterialInstance::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnMaterialInstance >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaMaterialInstance::GM_TYPELIB ) / sizeof( GaMaterialInstance::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaMaterialInstance::GM_TYPE, GaMaterialInstance::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaTexture
gmFunctionEntry GaTexture::GM_TYPELIB[] = 
{
	{ "GetWidth",				GaTexture::GetWidth },
	{ "GetHeight",				GaTexture::GetHeight },
	{ "GetTexel",				GaTexture::GetTexel }
};

int GM_CDECL GaTexture::GetWidth( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS(0);
	ScnTexture* pTexture = (ScnTexture*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushInt( (int)pTexture->getWidth() );
	
	return GM_OK;
}

int GM_CDECL GaTexture::GetHeight( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS(0);
	ScnTexture* pTexture = (ScnTexture*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushInt( (int)pTexture->getHeight() );
	
	return GM_OK;
}

int GM_CDECL GaTexture::GetTexel( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS(2);
	GM_CHECK_INT_PARAM( X, 0 );
	GM_CHECK_INT_PARAM( Y, 1 );
	ScnTexture* pTexture = (ScnTexture*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushNewUser( GaVec4::Alloc( a_thread->GetMachine(), pTexture->getTexel( X, Y ) ), GaVec4::GM_TYPE );

	return GM_OK;
}

void GM_CDECL GaTexture::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnTexture >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaTexture::GM_TYPELIB ) / sizeof( GaTexture::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaTexture::GM_TYPE, GaTexture::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaScript
gmFunctionEntry GaScript::GM_TYPELIB[] = 
{
	{ "Execute",				GaScript::Execute }
};

int GM_CDECL GaScript::Execute( gmThread* a_thread )
{
	ScnScript* pScript = (ScnScript*)a_thread->ThisUser_NoChecks();

	int ThreadID = -1;
	if( a_thread->GetNumParams() == 0 )
	{	
		ThreadID = pScript->execute( BcFalse );
	}
	else if( a_thread->GetNumParams() == 1 )
	{
		GM_CHECK_INT_PARAM( Now, 0 );
		ThreadID = pScript->execute( Now ? BcTrue : BcFalse );
	}
	
	a_thread->PushInt( ThreadID );
	
	return GM_OK;
}

void GM_CDECL GaScript::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnScript >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaScript::GM_TYPELIB ) / sizeof( GaScript::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaScript::GM_TYPE, GaScript::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaSound
gmFunctionEntry GaSound::GM_TYPELIB[] = 
{
	{ NULL, NULL }
};

void GM_CDECL GaSound::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnSound >::CreateType( a_machine );
	
	// Register type library.
	//int NoofEntries = sizeof( GaSound::GM_TYPELIB ) / sizeof( GaSound::GM_TYPELIB[0] );
	//a_machine->RegisterTypeLibrary( GaSound::GM_TYPE, GaSound::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaSoundEmitter
gmFunctionEntry GaSoundEmitter::GM_TYPELIB[] = 
{
	{ "Play",				GaSoundEmitter::Play }
};

int GM_CDECL GaSoundEmitter::Create( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_STRING_PARAM( pName, 0 )

	ScnSoundEmitterRef SoundEmitterRef;
	if( CsCore::pImpl()->createResource( pName, SoundEmitterRef ) )
	{
		// Allocate a new object instance for the font instance.
		gmUserObject* pUserObj = GaSoundEmitter::AllocUserObject( a_thread->GetMachine(), SoundEmitterRef );
		
		// Push onto stack.
		a_thread->PushUser( pUserObj );
		
		// Add resource block incase it isn't ready.
		return GaCore::pImpl()->addResourceBlock( SoundEmitterRef, pUserObj, a_thread ) ? GM_SYS_BLOCK : GM_OK;
	}
	
	return GM_OK;
}

int GM_CDECL GaSoundEmitter::Play( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( ScnSound*, GaSound::GM_TYPE, pSound, 0 );
	ScnSoundEmitter* pSoundEmitter = (ScnSoundEmitter*)a_thread->ThisUser_NoChecks();

	pSoundEmitter->play( pSound );
		
	return GM_OK;
}

void GM_CDECL GaSoundEmitter::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< ScnSoundEmitter >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaSoundEmitter::GM_TYPELIB ) / sizeof( GaSoundEmitter::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaSoundEmitter::GM_TYPE, GaSoundEmitter::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// GaFrame - Very quick and dirty way to access RsFrame for rendering.
gmType GaFrame::GM_TYPE = GM_NULL;

bool GM_CDECL GaFrame::Trace( gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workRemaining, int& a_workDone )
{
	// Do nothing.
	++a_workDone;
	return false;
}

void GM_CDECL GaFrame::Destruct( gmMachine* a_machine, gmUserObject* a_object )
{
	// Do nothing.
}

void GM_CDECL GaFrame::AsString( gmUserObject* a_object, char* a_buffer, int a_bufferLen )
{
	RsFrame* pObj = (RsFrame*)a_object->m_user;

	BcSPrintf( a_buffer, "<RsFrame Object @ %p>", pObj );
}

int GM_CDECL GaFrame::FrameBegin( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	
	RsFrame* pFrame = RsCore::pImpl()->allocateFrame();
	
	// TEMP: Setup default viewport.
	RsViewport Viewport( 0, 0, 1280, 720 );
	BcReal W = 1280.0f * 0.5f;
	BcReal H = 720.0f * 0.5f;
	
	pFrame->setRenderTarget( NULL );
	pFrame->setViewport( Viewport );
	
	a_thread->PushNewUser( pFrame, GM_TYPE );
	
	return GM_OK;
}

int GM_CDECL GaFrame::FrameEnd( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( RsFrame*, GaFrame::GM_TYPE, pFrame, 0 );
	
	RsCore::pImpl()->queueFrame( pFrame );
		
	// Yield so renderer picks up the frame next tick.
	return GM_SYS_YIELD;
}

void GM_CDECL GaFrame::CreateType( gmMachine* a_machine )
{
	GM_TYPE = a_machine->CreateUserType( "Frame" );
	
	a_machine->RegisterUserCallbacks( GaFrame::GM_TYPE,
									 &GaFrame::Trace,
									 &GaFrame::Destruct,
									 &GaFrame::AsString ); 
}

//////////////////////////////////////////////////////////////////////////
// gLibScene
static gmFunctionEntry gLibScene[] = 
{
	// Resource requesting.
	{ "Font",					GaFont::Request },
	{ "Material",				GaMaterial::Request },
	{ "Model",					GaModel::Request },
	{ "Package",				GaPackage::Request },
	{ "Shader",					GaShader::Request },
	{ "Texture",				GaTexture::Request },
	{ "Script",					GaScript::Request },
	{ "Sound",					GaSound::Request },
	
	// Resource creating.
	{ "Canvas",					GaCanvas::Create },
	{ "SoundEmitter",			GaSoundEmitter::Create },
	
	// Hacky RsFrame stuff.
	{ "FrameBegin",				GaFrame::FrameBegin },
	{ "FrameEnd",				GaFrame::FrameEnd },
};


//////////////////////////////////////////////////////////////////////////
// GaLibrarySceneBinder
void GaLibrarySceneBinder( gmMachine* a_machine )
{
	// Register types.
	GaFont::CreateType( a_machine );
	GaFontInstance::CreateType( a_machine );
	GaMaterial::CreateType( a_machine );
	GaMaterialInstance::CreateType( a_machine );
	GaModel::CreateType( a_machine );
	GaModelInstance::CreateType( a_machine );
	GaPackage::CreateType( a_machine );
	GaShader::CreateType( a_machine );
	GaTexture::CreateType( a_machine );
	GaScript::CreateType( a_machine );
	GaSound::CreateType( a_machine );
	GaSoundEmitter::CreateType( a_machine );

	// 
	GaCanvas::CreateType( a_machine );
	
	//
	GaFrame::CreateType( a_machine );
	
	// Register library.
	a_machine->RegisterLibrary( gLibScene, sizeof( gLibScene ) / sizeof( gLibScene[0] ), "Scene" );
}


