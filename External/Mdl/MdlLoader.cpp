/**************************************************************************
*
* File:		MdlLoader.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "MdlLoader.h"

#include "BcString.h"

//////////////////////////////////////////////////////////////////////////
// loadModel
MdlNode* MdlLoader::loadModel( const BcChar* Filename )
{
	if( BcStrStr( Filename, ".md5mesh" ) )
	{
		return loadMD5Mesh( Filename );
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// loadAnim
MdlAnim* MdlLoader::loadAnim( const BcChar* Filename )
{
	if( BcStrStr( Filename, ".md5anim" ) )
	{
		return loadMD5Anim( Filename );
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// loadMD5Mesh
#include "MD5MeshLoader.h"
MdlNode* MdlLoader::loadMD5Mesh( const BcChar* Filename )
{
	MD5MeshLoader Loader;
	return Loader.load( Filename, "root" );
}

//////////////////////////////////////////////////////////////////////////
// loadMD5Anim
#include "MD5AnimLoader.h"
MdlAnim* MdlLoader::loadMD5Anim( const BcChar* Filename )
{
	MD5AnimLoader Loader;
	return Loader.load( Filename, "root" );
}
