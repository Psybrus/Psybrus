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
#include "BcString.h"

//////////////////////////////////////////////////////////////////////////
// Snd
SndSound* Snd::load( const BcChar* pFileName )
{
	if( BcStrStr( pFileName, ".wav" ) != NULL )
	{
		return loadWav( pFileName );
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
