#pragma once

#include "Base/BcName.h"

//////////////////////////////////////////////////////////////////////////
// OsController
class OsController
{
public:
	OsController();
	~OsController();

	/**
	 * Add input device to controller.
	 * @return ID of input device.
	 */
	BcU32 addInputDevice( class OsInputDevice* InputDevice );

	/**
	 * Add primary binding.
	 * @param Name Name of binding.
	 * @param InputDeviceID Input device ID as returned by addInputDevice.
	 * @param InputID ID of input.
	 * @param Multiplier Value to multiply raw input by before processing.
	 * @return ID of binding.
	 */
	BcU32 addPrimaryBinding( BcName Name, BcU32 InputDeviceID, BcU32 InputID, BcF32 Multiplier = 1.0f );

	/**
	 * Add secondary binding.
	 * @param Name Name of binding.
	 * @param InputDeviceID Input device ID as returned by addInputDevice.
	 * @param InputID ID of input.
	 * @param Multiplier Value to multiply raw input by before processing.
	 * @return ID of binding.
	 */
	BcU32 addSecondaryBinding( BcName Name, BcU32 InputDeviceID, BcU32 InputID, BcF32 Multiplier = 1.0f );

	/**
	 * Get binding ID.
	 * @param Name Name of binding.
	 * @return Binding ID. BcErrorCode if invalid.
	 */
	BcU32 getBindingID( BcName Name ) const;

	/**
	 * @param BindingID
	 * @return State.
	 */
	BcF32 getState( BcU32 BindingID ) const;

	/**
	 * @param BindingID.
	 * @return Is binding down.
	 */
	bool isDown( BcU32 BindingID ) const;

	/**
	 * @param BindingID.
	 * @return Is binding up.
	 */
	bool isUp( BcU32 BindingID ) const;

	/**
	 * @param BindingID.
	 * @return Was binding pressed since last frame?
	 */
	bool wasPressed( BcU32 BindingID ) const;

	/**
	 * @param BindingID.
	 * @return Was binding released since last frame?
	 */
	bool wasReleased( BcU32 BindingID ) const;

	/**
	 * Update controller.
	 * Called internally by the OsCore at the beginning of each frame.
	 */
	void update();

private:
	struct Input
	{
		BcU32 InputDeviceID_ = BcErrorCode;
		BcU32 InputID_ = BcErrorCode;
		BcF32 Multiplier_ = 0.0f;
	};

	struct State
	{
		BcF32 Prev_ = 0.0f;
		BcF32 Curr_ = 0.0f;
	};

	struct Binding
	{
		BcName Name_ = BcName::INVALID;
		Input Primary_;
		Input Secondary_;
	};

	std::vector< class OsInputDevice* > InputDevices_;
	std::vector< Binding > Bindings_;
	std::vector< State > State_;
};
