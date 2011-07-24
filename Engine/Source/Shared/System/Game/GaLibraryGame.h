/**************************************************************************
*
* File:		GaLibraryGame.h
* Author:	Neil Richardson 
* Ver/Date:	24/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaLibraryGame_H__
#define __GaLibraryGame_H__

#include "BcTypes.h"

#include "GaLibrary.h"

//////////////////////////////////////////////////////////////////////////
// Game objects.
#include "GaPackage.h"
#include "GaScript.h"
#include "GaEntity.h"
#include "GaComponent.h"

class GaGamePackage: public GaLibraryResource< GaPackage >
{
public:
	
};

class GaGameScript: public GaLibraryResource< GaScript >
{
	static gmFunctionEntry GM_TYPELIB[];
	
public:
	static int GM_CDECL Execute( gmThread* a_thread );
	
	static void GM_CDECL CreateType( gmMachine* a_machine );
};

/*
class GaGameEntity: public GaLibraryResource< GaEntity >
{
public:
	
};

class GaGameComponent: public GaLibraryResource< GaComponent >
{
public:
	
};
 */

//////////////////////////////////////////////////////////////////////////
// GaLibraryGameBinder
void GaLibraryGameBinder( class gmMachine* pGmMachine );


#endif


