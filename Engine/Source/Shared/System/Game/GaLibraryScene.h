/**************************************************************************
*
* File:		GaLibraryScene.h
* Author:	Neil Richardson 
* Ver/Date:	21/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaLibraryScene_H__
#define __GaLibraryScene_H__

#include "BcTypes.h"

#include "GaLibrary.h"

//////////////////////////////////////////////////////////////////////////
// Scene objects.
#include "ScnCanvas.h"
#include "ScnFont.h"
#include "ScnMaterial.h"
#include "ScnModel.h"
#include "ScnShader.h"
#include "ScnTexture.h"
#include "ScnTextureAtlas.h"
#include "ScnSound.h"
#include "ScnSoundEmitter.h"

//////////////////////////////////////////////////////////////////////////
// GaLibraryResource types.
class GaSceneCanvas: public GaLibraryResource< ScnCanvas >
{
public:
	static gmFunctionEntry GM_TYPELIB[];

public:
	static int GM_CDECL Create( gmThread* a_thread );

	static int GM_CDECL SetMaterialInstance( gmThread* a_thread );
	static int GM_CDECL PushMatrix( gmThread* a_thread );
	static int GM_CDECL PopMatrix( gmThread* a_thread );
	
	static int GM_CDECL DrawSprite( gmThread* a_thread );
	static int GM_CDECL DrawSpriteCentered( gmThread* a_thread );

	static int GM_CDECL Clear( gmThread* a_thread );
	static int GM_CDECL Render( gmThread* a_thread );

	static void GM_CDECL CreateType( gmMachine* a_machine );

};

class GaSceneFont: public GaLibraryResource< ScnFont >
{
public:
	static gmFunctionEntry GM_TYPELIB[];
	
public:
	static int GM_CDECL CreateInstance( gmThread* a_thread );

	static void GM_CDECL CreateType( gmMachine* a_machine );
};

class GaSceneFontInstance: public GaLibraryResource< ScnFontInstance >
{
public:
	static gmFunctionEntry GM_TYPELIB[];

public:
	static int GM_CDECL Draw( gmThread* a_thread );
	static int GM_CDECL GetMaterialInstance( gmThread* a_thread );

	static void GM_CDECL CreateType( gmMachine* a_machine );
};

class GaSceneMaterial: public GaLibraryResource< ScnMaterial >
{
public:
	static gmFunctionEntry GM_TYPELIB[];

public:
	static int GM_CDECL CreateInstance( gmThread* a_thread );
	
	static void GM_CDECL CreateType( gmMachine* a_machine );
};

class GaSceneMaterialInstance: public GaLibraryResource< ScnMaterialInstance >
{
public:
	static gmFunctionEntry GM_TYPELIB[];

public:
	static int GM_CDECL FindParameter( gmThread* a_thread );
	static int GM_CDECL SetParameter( gmThread* a_thread );
	static int GM_CDECL GetTexture( gmThread* a_thread );
	
	static void GM_CDECL CreateType( gmMachine* a_machine );
};

class GaSceneModel: public GaLibraryResource< ScnModel >
{
public:
	
};

class GaSceneModelInstance: public GaLibraryResource< ScnModelInstance >
{
public:
	
};

class GaSceneShader: public GaLibraryResource< ScnShader >
{
public:
	
};

class GaSceneTexture: public GaLibraryResource< ScnTexture >
{
public:
	static gmFunctionEntry GM_TYPELIB[];
	
public:
	static int GM_CDECL GetWidth( gmThread* a_thread );
	static int GM_CDECL GetHeight( gmThread* a_thread );
	static int GM_CDECL GetTexel( gmThread* a_thread );
	
	static void GM_CDECL CreateType( gmMachine* a_machine );
};

class GaSceneSound: public GaLibraryResource< ScnSound >
{
public:
	static gmFunctionEntry GM_TYPELIB[];
	
public:
	static void GM_CDECL CreateType( gmMachine* a_machine );
};

class GaSceneSoundEmitter: public GaLibraryResource< ScnSoundEmitter >
{
public:
	static gmFunctionEntry GM_TYPELIB[];
	
public:
	static int GM_CDECL Create( gmThread* a_thread );
	static int GM_CDECL Play( gmThread* a_thread );
	
	static void GM_CDECL CreateType( gmMachine* a_machine );
};

//////////////////////////////////////////////////////////////////////////
// GaSceneFrame - Very quick and dirty way to access RsFrame for rendering.
//           This will be done via a scene manager and view later on.
class GaSceneFrame
{
public:
	static gmType GM_TYPE;
	
public:
	static bool GM_CDECL Trace( gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workRemaining, int& a_workDone );
	static void GM_CDECL Destruct( gmMachine* a_machine, gmUserObject* a_object );
	static void GM_CDECL AsString( gmUserObject* a_object, char* a_buffer, int a_bufferLen );

	static int GM_CDECL FrameBegin( gmThread* a_thread );
	static int GM_CDECL FrameEnd( gmThread* a_thread );

	static void GM_CDECL CreateType( gmMachine* a_machine );
};

//////////////////////////////////////////////////////////////////////////
// GaLibrarySceneBinder
void GaLibrarySceneBinder( class gmMachine* pGmMachine );

#endif


