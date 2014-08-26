/**************************************************************************
*
* File:		Img.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*
*		
*
*
* 
**************************************************************************/

#ifndef __IMG_H__
#define __IMG_H__

//////////////////////////////////////////////////////////////////////////
// Includes
#include "ImgTypes.h"
#include "ImgImage.h"

//////////////////////////////////////////////////////////////////////////
// Img
class Img
{
public:
	static ImgImageUPtr		load( const BcChar* Filename );
	static BcBool			save( const BcChar* Filename, ImgImage* pImage );

public:
	static ImgImageUPtr		loadPNG( const BcChar* Filename );
	static BcBool			savePNG( const BcChar* Filename, ImgImage* pImage );
};

#endif
