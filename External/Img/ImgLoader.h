/**************************************************************************
*
* File:		ImgLoader.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*
*		
*
*
* 
**************************************************************************/

#ifndef __IMGLOADER_H__
#define __IMGLOADER_H__

//////////////////////////////////////////////////////////////////////////
// Includes
#include "ImgImage.h"

//////////////////////////////////////////////////////////////////////////
// ImgLoader
class ImgLoader
{
public:
	static ImgImage*		load( const BcChar* Filename );

	static ImgImage*		loadPNG( const BcChar* Filename );
};

#endif
