/**************************************************************************
*
* File:		MdlLoader.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __MDLLOADER_H__
#define __MDLLOADER_H__

#include "MdlTypes.h"
#include "MdlAnim.h"
#include "MdlNode.h"

//////////////////////////////////////////////////////////////////////////
// MdlLoader
class MdlLoader
{
public:
	static MdlNode*			loadModel( const BcChar* Filename );
	static MdlAnim*			loadAnim( const BcChar* Filename );
	
	static MdlNode*		loadMD5Mesh( const BcChar* Filename );
	static MdlAnim*		loadMD5Anim( const BcChar* Filename );
};

#endif