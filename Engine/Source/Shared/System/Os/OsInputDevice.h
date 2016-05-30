#pragma once

#include "Base/BcName.h"

#include "Events/EvtPublisher.h"
#include "Math/MaVec2d.h"
#include "Math/MaVec3d.h"

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
	 * Is connected?
	 */
	virtual bool isConnected() const = 0;

	/**
	 * Get player ID.
	 * Xbox or Playstation controllers have lights
	 * indicate player ID. 0 = Player 1, 1 = Player 2, etc.
	 */
	virtual BcU32 getPlayerID() const = 0;

	/**
	 * @return Number of inputs.
	 * This may include some invalid inputs inside of its range.
	 */
	virtual BcU32 getNoofInputs() const = 0;

	/**
	 * @return Input name. BcName::INVALID if ID not valid.
	 */
	virtual BcName getInputName( BcU32 ID ) const = 0;

	/**
	 * Get input ID.
	 */
	virtual BcU32 getInputID( BcName Name ) const = 0;

	/**
	 * Get float input.
	 * @param ID ID of input.
	 */
	virtual BcF32 getFloatInput( BcU32 ID ) const = 0;

	/**
	 * Get vec2 input.
	 * @param ID ID of first input.
	 * @return Vector of (ID, ID + 1)
	 */
	virtual MaVec2d getVec2Input( BcU32 ID ) const = 0;

	/**
	 * Get vec3 input.
	 * @param ID ID of first input.
	 * @return Vector of (ID, ID + 1, ID + 2)
	 */
	virtual MaVec3d getVec3Input( BcU32 ID ) const = 0;

	/**
	 * Get bool input.
	 * @param ID ID of input.
	 */
	virtual bool getBoolInput( BcU32 ID ) const = 0;

	/**
	 * Get int input.
	 * @param ID ID of input.
	 */
	virtual BcS64 getIntInput( BcU32 ID ) const = 0;

	/**
	 * Get text input. All accumulated since previous frame.
	 * @param ID ID of input.
	 * @return Pointer to string. Only valid until end of frame.
	 */
	virtual const char* getTextInput( BcU32 ID ) const = 0;

	/**
	 * Update input device.
	 * Called internally by the OsCore at the beginning of each frame.
	 */
	virtual void update() = 0;
};
