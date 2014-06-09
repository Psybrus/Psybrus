/**************************************************************************
*
* File:		ScnRenderPipeline.h
* Author:	Neil Richardson 
* Ver/Date:		
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnRenderPipeline_H__
#define __ScnRenderPipeline_H__

#include "System/Renderer/RsCore.h"
#include "System/Content/CsResource.h"

//////////////////////////////////////////////////////////////////////////
// ScnRenderPipeline
class ScnRenderPipeline:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnRenderPipeline );

public:
	void initialise();
	virtual void destroy();

protected:

};

#endif
