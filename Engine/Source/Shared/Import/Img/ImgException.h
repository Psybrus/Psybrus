/**************************************************************************
*
* File:		ImgException.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Exception.
*		
*
*
* 
**************************************************************************/

#ifndef __IMGEXCEPTION_H__
#define __IMGEXCEPTION_H__

#include "ImgTypes.h"

#include <string>

//////////////////////////////////////////////////////////////////////////
// ImgException
class ImgException
{
public:
	ImgException( const BcChar* pWhat ):
		What_( pWhat )
	{
	
	}
	
	const BcChar* what() const
	{
		return What_.c_str();
	}
	
private:
	std::string What_;
};

#endif
