#include "System/Os/OsInputDeviceXInputWindows.h"

#include "System/Os/OsCore.h"

//////////////////////////////////////////////////////////////////////////
// Static Data
namespace
{
	BcName InputType_( "XINPUT" );
	std::array< BcName, OsInputDeviceXInputWindows::INPUT_MAX > InputNames_ =
	{
		"BUTTON_DPAD_U",
		"BUTTON_DPAD_D",
		"BUTTON_DPAD_L",
		"BUTTON_DPAD_R",
		"BUTTON_START",
		"BUTTON_BACK",
		"BUTTON_THUMB_L",
		"BUTTON_THUMB_R",
		"BUTTON_SHOULDER_L",
		"BUTTON_SHOULDER_R",
		"BUTTON_A",
		"BUTTON_B",
		"BUTTON_X",
		"BUTTON_Y",
		"TRIGGER_L",
		"TRIGGER_R",
		"THUMB_LX",
		"THUMB_LY",
		"THUMB_RX",
		"THUMB_RY",
	};

	BcF32 NormaliseInput( BcS32 InputValue, BcS32 Deadzone, BcS32 Max )
	{
		if( InputValue > Deadzone)
		{
			return static_cast< BcF32 >( InputValue - Deadzone ) / static_cast< BcF32 >( Max - Deadzone );
		}
		else if( InputValue < -Deadzone )
		{
			return static_cast< BcF32 >( InputValue + Deadzone ) / static_cast< BcF32 >( Max - Deadzone );
		}
		return 0.0;
	}

	BcF32 GetButton( BcU32 Buttons, BcU32 Button )
	{
		return ( Buttons & Button ) != 0 ? 1.0f : 0.0f;
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
OsInputDeviceXInputWindows::OsInputDeviceXInputWindows( BcU32 UserIdx ):
	UserIdx_( UserIdx ),
	IsConnected_( false )
{
	memset( &InputState_, 0, sizeof( InputState_ ) );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsInputDeviceXInputWindows::~OsInputDeviceXInputWindows()
{
}

//////////////////////////////////////////////////////////////////////////
// getTypeName
BcName OsInputDeviceXInputWindows::getTypeName() const
{
	return InputType_;
}

//////////////////////////////////////////////////////////////////////////
// isConnected
bool OsInputDeviceXInputWindows::isConnected() const
{
	return IsConnected_;
}

//////////////////////////////////////////////////////////////////////////
// getPlayerID
BcU32 OsInputDeviceXInputWindows::getPlayerID() const
{
	return UserIdx_;
}

//////////////////////////////////////////////////////////////////////////
// getNoofInputs
BcU32 OsInputDeviceXInputWindows::getNoofInputs() const
{
	return INPUT_MAX;
}

//////////////////////////////////////////////////////////////////////////
// getInputName
BcName OsInputDeviceXInputWindows::getInputName( BcU32 ID ) const
{
	if( ID < INPUT_MAX )
	{
		return InputNames_[ ID ];
	}
	return BcName::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// getInputID
BcU32 OsInputDeviceXInputWindows::getInputID( BcName Name ) const
{
	auto It = std::find( InputNames_.begin(), InputNames_.end(), Name );
	if( It != InputNames_.end() )
	{
		return static_cast< BcU32 >( It - InputNames_.begin() );
	}
	return BcErrorCode;

}

//////////////////////////////////////////////////////////////////////////
// getFloatInput
BcF32 OsInputDeviceXInputWindows::getFloatInput( BcU32 ID ) const
{
	switch( ID )
	{
	case BUTTON_DPAD_U:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP );
	case BUTTON_DPAD_D:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN );
	case BUTTON_DPAD_L:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT );
	case BUTTON_DPAD_R:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT );
	case BUTTON_START:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_START );
	case BUTTON_BACK:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_BACK );
	case BUTTON_THUMB_L:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB );
	case BUTTON_THUMB_R:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB );
	case BUTTON_SHOULDER_L:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER );
	case BUTTON_SHOULDER_R:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER );
	case BUTTON_A:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_A );
	case BUTTON_B:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_B );
	case BUTTON_X:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_X );
	case BUTTON_Y:
		return GetButton( InputState_.Gamepad.wButtons, XINPUT_GAMEPAD_Y );
	case TRIGGER_L:
		return NormaliseInput( InputState_.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255 );
	case TRIGGER_R:
		return NormaliseInput( InputState_.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255 );
	case THUMB_LX:
		return NormaliseInput( InputState_.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, 32767 );
	case THUMB_LY:
		return NormaliseInput( InputState_.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, 32767 );
	case THUMB_RX:
		return NormaliseInput( InputState_.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, 32767 );
	case THUMB_RY:
		return NormaliseInput( InputState_.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, 32767 );
	}

	return 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// getVec2Input
MaVec2d OsInputDeviceXInputWindows::getVec2Input( BcU32 ID ) const
{
	return MaVec2d( getFloatInput( ID ), getFloatInput( ID + 1 ) );
}

//////////////////////////////////////////////////////////////////////////
// getVec3Input
MaVec3d OsInputDeviceXInputWindows::getVec3Input( BcU32 ID ) const
{
	return MaVec3d( getFloatInput( ID ), getFloatInput( ID + 1 ), getFloatInput( ID + 2 ) );
}

//////////////////////////////////////////////////////////////////////////
// getBoolInput
bool OsInputDeviceXInputWindows::getBoolInput( BcU32 ID ) const
{
	return getFloatInput( ID ) > 0.5f;
}

//////////////////////////////////////////////////////////////////////////
// getIntInput
BcS64 OsInputDeviceXInputWindows::getIntInput( BcU32 ID ) const
{
	return static_cast< BcS64 >( getFloatInput( ID ) );
}

//////////////////////////////////////////////////////////////////////////
// getTextInput
const char* OsInputDeviceXInputWindows::getTextInput( BcU32 ID ) const
{
	return "";
}

//////////////////////////////////////////////////////////////////////////
// update
void OsInputDeviceXInputWindows::update()
{
	auto RetVal = XInputGetState( UserIdx_, &InputState_ );
	if( RetVal == ERROR_DEVICE_NOT_CONNECTED )
	{
		memset( &InputState_, 0, sizeof( InputState_ ) );
		IsConnected_ = false;
	}
	else
	{
		IsConnected_ = true;
	}
}

//////////////////////////////////////////////////////////////////////////
// setInput
void OsInputDeviceXInputWindows::setInput( BcU32 Input, BcU32 Offset, BcF32 Value )
{

}
