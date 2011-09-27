/**************************************************************************
*
* File:		GaLibraryGame.cpp
* Author:	Neil Richardson 
* Ver/Date:	24/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaLibraryGame.h"

//////////////////////////////////////////////////////////////////////////
// GaGameScript
gmFunctionEntry GaGameScript::GM_TYPELIB[] = 
{
	{ "Execute",				GaGameScript::Execute }
};

int GM_CDECL GaGameScript::Execute( gmThread* a_thread )
{
	GaScript* pScript = (GaScript*)a_thread->ThisUser_NoChecks();
	
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

void GM_CDECL GaGameScript::CreateType( gmMachine* a_machine )
{
	GaLibraryResource< GaScript >::CreateType( a_machine );
	
	// Register type library.
	int NoofEntries = sizeof( GaGameScript::GM_TYPELIB ) / sizeof( GaGameScript::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaGameScript::GM_TYPE, GaGameScript::GM_TYPELIB, NoofEntries );
}

//////////////////////////////////////////////////////////////////////////
// gLibScene
static gmFunctionEntry gLibGame[] =  
{
	// Resource requesting.
	{ "Package",				GaGamePackage::Request },
	{ "Script",					GaGameScript::Request },
	
	// Resource creating.
	
};

//////////////////////////////////////////////////////////////////////////
// GaLibraryGameBinder
void GaLibraryGameBinder( gmMachine* a_machine )
{
	// Register types.
	GaGamePackage::CreateType( a_machine );
	GaGameScript::CreateType( a_machine );
	
	// Register library.
	a_machine->RegisterLibrary( gLibGame, sizeof( gLibGame ) / sizeof( gLibGame[0] ), "Game" );
}
