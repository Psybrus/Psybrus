/**************************************************************************
*
* File:		RsShader.h
* Author:	Neil Richardson 
* Ver/Date:	4/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSSHADER_H__
#define __RSSHADER_H__

#include "RsTypes.h"
#include "RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsShader
class RsShader:
	public RsResource
{
public:
	RsShader();
	virtual ~RsShader();
};

#endif
