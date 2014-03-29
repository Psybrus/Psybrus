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

#include "System/Renderer/RsTypes.h"
#include "System/SysResource.h"

////////////////////////////////////////////////////////////////////////////////
// RsResource
class RsResource:
	public SysResource
{
public:
	RsResource( class RsContext* pContext );
	virtual ~RsResource();

	/**
	 * Get context resource belongs to.
	 */
	class RsContext*				getContext();

private:
	class RsContext*				pContext_;

};

#endif
