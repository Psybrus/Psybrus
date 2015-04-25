/**************************************************************************
*
* File:		OsClient.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Operating system client.
*		
*
*
* 
**************************************************************************/

#ifndef __OSCLIENT_H__
#define __OSCLIENT_H__

#include "OsEvents.h"
#include "Events/EvtPublisher.h"

//////////////////////////////////////////////////////////////////////////
// OsClient
class OsClient:
	public EvtPublisher
{
public:
	OsClient();
	virtual ~OsClient();

	/**
	 * Update.
	 */
	virtual void update() = 0;

	/**
 	 * Get device handle.
	 */
	virtual BcHandle getDeviceHandle() = 0;

	/**
 	 * Get window handle.
	 */
	virtual BcHandle getWindowHandle() = 0;

	/**
 	 * Get width.
	 */
	virtual BcU32 getWidth() const = 0;

	/**
 	 * Get height.
	 */
	virtual BcU32 getHeight() const = 0;

	/**
	 * Set mouse lock.
	 */
	virtual void setMouseLock( BcBool Enabled ) = 0;

};

#endif
