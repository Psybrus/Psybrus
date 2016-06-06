/**************************************************************************
*
* File:		OsCore.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OSCORE_H__
#define __OSCORE_H__

#include "Base/BcGlobal.h"
#include "System/SysSystem.h"

#include "OsController.h"
#include "OsClient.h"
#include "OsEvents.h"
#include "OsInputDevice.h"

//////////////////////////////////////////////////////////////////////////
// OsCore
class OsCore:
	public BcGlobal< OsCore >,
	public SysSystem
{
public:
	OsCore();
	virtual ~OsCore();

	/**
	 * Update.
	 */
	virtual void update();

	/**
	 * Register client.
	 */
	void registerClient( OsClient* pClient );

	/**
	 * Unregister client.
	 */
	void unregisterClient( OsClient* pClient );

	/**
	 * Register input device.
	 */
	void registerInputDevice( class OsInputDevice* InputDevice );
	
	/**
	 * Unregister input device.
	 */
	void unregisterInputDevice( class OsInputDevice* InputDevice );

	/**
 	 * Get number of clients.
	 */
	size_t getNoofClients() const;

	/**
 	 * Get number of input devices.
	 */
	size_t getNoofInputDevices() const;

	/**
	 * Get client.
	 */
	OsClient* getClient( size_t Index ) const;

	/**
	 * Get input device.
	 */
	OsInputDevice* getInputDevice( size_t Index ) const;
	OsInputDevice* getInputDevice( BcName TypeName, BcU32 PlayerID ) const;


protected:
	std::vector< OsInputDevice* > InputDeviceList_;
	std::vector< OsClient* > ClientList_;

};

#endif

