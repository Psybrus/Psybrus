/**************************************************************************
*
* File:		ScnFont.cpp
* Author:	Neil Richardson 
* Ver/Date:	16/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnFont.h"

#include "CsCore.h"

#ifdef PSY_SERVER
#include "BcFile.h"
#include "BcStream.h"
#include "json.h"
#endif

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnMaterial::import( const Json::Value& Object )
{
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnFont );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnFont::initialise()
{
	
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnFont::create()
{
	
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnFont::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnFont::isReady()
{
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnFont::fileReady()
{
	// File is ready, get the header chunk.
	pFile_->getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnFont::fileChunkReady( const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		
	}
}

