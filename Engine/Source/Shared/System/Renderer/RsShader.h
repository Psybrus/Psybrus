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

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsShader
class RsShader:
	public RsResource
{
public:
	RsShader( class RsContext* pContext );
	virtual ~RsShader();
};

#endif
