/**************************************************************************
*
* File:		RsResource.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Renderer resource.
*		
*
*
* 
**************************************************************************/

#ifndef __RSRESOURCE_H__
#define __RSRESOURCE_H__

#include "RsTypes.h"
#include "SysResource.h"

////////////////////////////////////////////////////////////////////////////////
// RsResource
class RsResource:
	public SysResource
{
public:
	RsResource();
	virtual ~RsResource();

};

#endif
