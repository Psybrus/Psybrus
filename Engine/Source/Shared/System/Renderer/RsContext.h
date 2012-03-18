/**************************************************************************
*
* File:		RsContext.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCONTEXT_H__
#define __RSCONTEXT_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class OsClient;

//////////////////////////////////////////////////////////////////////////
// RsContext
class RsContext:
	public RsResource
{
public:
	RsContext( OsClient* pClient );
	virtual ~RsContext();

	/**
 	 * Get width.
	 */
	virtual BcU32 getWidth() const = 0;

	/**
 	 * Get height.
	 */
	virtual BcU32 getHeight() const = 0;

	/**
	 * Requests a screenshot from the context.
	 */
	virtual void takeScreenshot() = 0;

protected:
	OsClient* pClient_;

private:

};


#endif
