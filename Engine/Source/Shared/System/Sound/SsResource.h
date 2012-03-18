/**************************************************************************
*
* File:		SsResource.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Sound resource.
*		
*
*
* 
**************************************************************************/

#ifndef __SSRESOURCE_H__
#define __SSRESOURCE_H__

#include "System/Sound/SsTypes.h"
#include "System/SysResource.h"

////////////////////////////////////////////////////////////////////////////////
// RsResource
class SsResource:
		public SysResource
{
public:
	SsResource();
	virtual ~SsResource();
};

#endif
