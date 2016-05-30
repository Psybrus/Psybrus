#pragma once

#include "System/Os/OsInputDevice.h"
#include "System/Os/OsWindows.h"

#include <Xinput.h>

//////////////////////////////////////////////////////////////////////////
// OsInputDeviceXInputWindows
class OsInputDeviceXInputWindows:
	public OsInputDevice
{
public:
	enum
	{
		// Buttons.
		BUTTON_DPAD_U = 0,
		BUTTON_DPAD_D,
		BUTTON_DPAD_L,
		BUTTON_DPAD_R,
		BUTTON_START,
		BUTTON_BACK,
		BUTTON_THUMB_L,
		BUTTON_THUMB_R,
		BUTTON_SHOULDER_L,
		BUTTON_SHOULDER_R,
		BUTTON_A,
		BUTTON_B,
		BUTTON_X,
		BUTTON_Y,

		// Analog.
		TRIGGER_L,
		TRIGGER_R,
		THUMB_LX,
		THUMB_LY,
		THUMB_RX,
		THUMB_RY,

		//
		INPUT_MAX
	};

public:
	OsInputDeviceXInputWindows( BcU32 UserIdx );
	virtual ~OsInputDeviceXInputWindows();

	BcName getTypeName() const override;
	BcU32 getPlayerID() const override;
	BcU32 getNoofInputs() const override;
	BcName getInputName( BcU32 ID ) const override;
	BcU32 getInputID( BcName Name ) const override;
	BcF32 getFloatInput( BcU32 ID ) const override;
	MaVec2d getVec2Input( BcU32 ID ) const override;
	MaVec3d getVec3Input( BcU32 ID ) const override;
	bool getBoolInput( BcU32 ID ) const override;
	BcS64 getIntInput( BcU32 ID ) const override;
	const char* getTextInput( BcU32 ID ) const override;

	void update() override;
	void setInput( BcU32 Input, BcU32 Offset, BcF32 Value );

private:
	BcU32 UserIdx_;
	XINPUT_STATE InputState_;
};
