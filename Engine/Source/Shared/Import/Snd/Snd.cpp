/**************************************************************************
*
* File:		Snd.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "Snd.h"

#include "SndFileWAV.h"
#include "SndFileOGG.h"
#include "Base/BcString.h"

//////////////////////////////////////////////////////////////////////////
// Snd
SndSound* Snd::load( const BcChar* pFileName )
{
	if( BcStrStr( pFileName, ".wav" ) != NULL )
	{
		return loadWav( pFileName );
	}
	else if( BcStrStr( pFileName, ".ogg" ) != NULL )
	{
		return loadOgg( pFileName );
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// Snd
SndSound* Snd::loadWav( const BcChar* pFileName )
{
	SndFileWAV* pFile = new SndFileWAV();
	
	if( pFile->load( pFileName ) )
	{
		return pFile;
	}
	
	delete pFile;
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// Snd
SndSound* Snd::loadOgg( const BcChar* pFileName )
{
	SndFileOGG* pFile = new SndFileOGG();
	
	if( pFile->load( pFileName ) )
	{
		return pFile;
	}
	
	delete pFile;
	return NULL;
}
