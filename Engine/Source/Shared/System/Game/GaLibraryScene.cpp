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
#include "GaLibrary.h"

#include "GaCore.h"

//////////////////////////////////////////////////////////////////////////
// Scene objects.
#include "ScnCanvas.h"
#include "ScnFont.h"
#include "ScnMaterial.h"
#include "ScnModel.h"
#include "ScnPackage.h"
#include "ScnShader.h"
#include "ScnTexture.h"

#include "GaLibrary.h"

//////////////////////////////////////////////////////////////////////////
// GaLibraryResource types.
template class GaLibraryResource< ScnCanvas >;

template class GaLibraryResource< ScnFont >;
template class GaLibraryResource< ScnMaterial >;
template class GaLibraryResource< ScnModel >;
template class GaLibraryResource< ScnPackage >;
template class GaLibraryResource< ScnShader >;
template class GaLibraryResource< ScnTexture >;

template class GaLibraryResource< ScnFontInstance >;
template class GaLibraryResource< ScnMaterialInstance >;
template class GaLibraryResource< ScnModelInstance >;

//////////////////////////////////////////////////////////////////////////
// gLibScene
static gmFunctionEntry gLibScene[] = 
{
	// Resource requesting.
	{ "Font",					&GaLibraryResource< ScnFont >::Request },
	{ "Material",				&GaLibraryResource< ScnMaterial >::Request },
	{ "Model",					&GaLibraryResource< ScnModel >::Request },
	{ "Package",				&GaLibraryResource< ScnPackage >::Request },
	{ "Shader",					&GaLibraryResource< ScnShader >::Request },
	{ "Texture",				&GaLibraryResource< ScnTexture >::Request },
	
	//
};

//////////////////////////////////////////////////////////////////////////
// GaLibrarySceneBinder
void GaLibrarySceneBinder( gmMachine* a_machine )
{
	// Register types.
	GaLibraryResource< ScnFont >::CreateType( a_machine );
	GaLibraryResource< ScnMaterial >::CreateType( a_machine );
	GaLibraryResource< ScnModel >::CreateType( a_machine );
	GaLibraryResource< ScnPackage >::CreateType( a_machine );
	GaLibraryResource< ScnShader >::CreateType( a_machine );
	GaLibraryResource< ScnTexture >::CreateType( a_machine );

	// Register library.
	a_machine->RegisterLibrary( gLibScene, sizeof( gLibScene ) / sizeof( gLibScene[0] ), "Scene" );
}


