/**************************************************************************
*
* File:		RsResourceInterface.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSRESOURCEINTERFACE_H__
#define __RSRESOURCEINTERFACE_H__

#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsResourceInterface
class RsResourceInterface
{
public:
	virtual ~RsResourceInterface(){};

	/**
	 * Create buffer.
	 * @param Buffer Buffer to create from.
	 */
	virtual bool createBuffer( 
		class RsBuffer* Buffer ) = 0;

	/**
	 * Destroy buffer.
	 * @param Buffer Buffer to destroy from.
	 */
	virtual bool destroyBuffer( 
		class RsBuffer* Buffer ) = 0;

	/**
	 * Update buffer.
	 * @param Buffer to update.
	 * @param Offset to update.
	 * @param Size to update.
	 * @param Flags for update.
	 * @param UpdateFunc to call.
	 */
	virtual bool updateBuffer( 
		class RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsBufferUpdateFlags Flags,
		RsUpdateBufferFunc UpdateFunc ) = 0;
};

#endif
