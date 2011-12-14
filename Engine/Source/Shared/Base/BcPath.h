/**************************************************************************
*
* File:		BcPath.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		File system path
*		
*
*
* 
**************************************************************************/

#ifndef __BCPATH_H__
#define __BCPATH_H__

#include "BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// BcPath
class BcPath
{
public:
	static const BcChar* BcPath::Seperators[2];

public:
	BcPath();
	BcPath( const std::string& Value );
	BcPath( const BcChar* pValue );
	
private:
	std::string InternalValue_;

};


#endif
