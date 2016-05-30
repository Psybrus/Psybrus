#pragma once

#include "Base/BcName.h"

#include "Events/EvtPublisher.h"

//////////////////////////////////////////////////////////////////////////
// OsInputDevice
class OsInputDevice:
	public EvtPublisher
{
public:
	OsInputDevice();
	virtual ~OsInputDevice();

	/**
	 * Get type name.
	 */
	virtual BcName getTypeName() const = 0;

	/**
	 * Get player ID.
	 * Xbox or Playstation controllers have lights
	 * indicate player ID. 0 = Player 1, 1 = Player 2, etc.
	 */
	virtual size_t getPlayerID() const = 0;

	/**
	 * Get number of controls.
	 */
	virtual size_t getNoofControls() const = 0;

	/**
	 * Get control name.
	 */
	virtual BcName getControlName( size_t ID ) const = 0;

	/**
	 * Get control ID.
	 */
	virtual size_t getControlID( BcName Name ) const = 0;

	/**
	 * Get float value.
	 * @param ID ID of control.
	 */
	virtual BcF32 getFloatValue( size_t ID ) const = 0;

	/**
	 * Get bool value.
	 * @param ID ID of control.
	 */
	virtual bool getBoolValue( size_t ID ) const = 0;

	/**
	 * Update input device.
	 * Called internally by the OsCore at the beginning of each frame.
	 */
	virtual void update() = 0;
};
