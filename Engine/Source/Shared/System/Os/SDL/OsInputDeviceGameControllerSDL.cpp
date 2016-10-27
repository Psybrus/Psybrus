#include "System/Os/SDL/OsInputDeviceGameControllerSDL.h"

#include "System/Os/OsCore.h"

//////////////////////////////////////////////////////////////////////////
// Static Data
namespace
{
	BcName InputType_( "JOYSTICK" );
	std::array< BcName, OsInputDeviceGameControllerSDL::INPUT_MAX > InputNames_ =
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
OsInputDeviceGameControllerSDL::OsInputDeviceGameControllerSDL( BcU32 JoystickIdx ):
	IsConnected_( false ),
	GameController_( nullptr ),
	Joystick_( nullptr ),
	Haptic_( nullptr ),
	HapticEffect_(),
	HapticEffectID_()
{
	memset( &HapticEffect_, 0, sizeof( HapticEffect_ ) );
	GameController_ = SDL_GameControllerOpen( JoystickIdx );
	Joystick_ = SDL_JoystickOpen( JoystickIdx );
	if( SDL_JoystickIsHaptic( Joystick_ ) )
	{
		Haptic_ = SDL_HapticOpenFromJoystick( Joystick_ );
		HapticEffect_.type = SDL_HAPTIC_LEFTRIGHT;
		HapticEffect_.leftright.length = 1000; // 1 second per update.
		HapticEffectID_ = SDL_HapticNewEffect( Haptic_, &HapticEffect_ );
		SDL_HapticRunEffect( Haptic_, HapticEffectID_, SDL_HAPTIC_INFINITY );
	}

	PrevInputValues_.fill( 0.0f );
	CurrInputValues_.fill( 0.0f );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsInputDeviceGameControllerSDL::~OsInputDeviceGameControllerSDL()
{
	if( Haptic_ )
	{
		SDL_HapticClose( Haptic_ );
	}
	SDL_JoystickClose( Joystick_ );
	SDL_GameControllerClose( GameController_ );
}

//////////////////////////////////////////////////////////////////////////
// getTypeName
BcName OsInputDeviceGameControllerSDL::getTypeName() const
{
	return InputType_;
}

//////////////////////////////////////////////////////////////////////////
// isConnected
bool OsInputDeviceGameControllerSDL::isConnected() const
{
	return IsConnected_;
}

//////////////////////////////////////////////////////////////////////////
// getPlayerID
BcU32 OsInputDeviceGameControllerSDL::getPlayerID() const
{
	return SDL_JoystickInstanceID( Joystick_ );
}

//////////////////////////////////////////////////////////////////////////
// getNoofInputs
BcU32 OsInputDeviceGameControllerSDL::getNoofInputs() const
{
	return INPUT_MAX;
}

//////////////////////////////////////////////////////////////////////////
// getInputName
BcName OsInputDeviceGameControllerSDL::getInputName( BcU32 ID ) const
{
	if( ID < INPUT_MAX )
	{
		return InputNames_[ ID ];
	}
	return BcName::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// getInputID
BcU32 OsInputDeviceGameControllerSDL::getInputID( BcName Name ) const
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
BcF32 OsInputDeviceGameControllerSDL::getFloatInput( BcU32 ID ) const
{
#if 0
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
#endif

	return 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// getVec2Input
MaVec2d OsInputDeviceGameControllerSDL::getVec2Input( BcU32 ID ) const
{
	return MaVec2d( getFloatInput( ID ), getFloatInput( ID + 1 ) );
}

//////////////////////////////////////////////////////////////////////////
// getVec3Input
MaVec3d OsInputDeviceGameControllerSDL::getVec3Input( BcU32 ID ) const
{
	return MaVec3d( getFloatInput( ID ), getFloatInput( ID + 1 ), getFloatInput( ID + 2 ) );
}

//////////////////////////////////////////////////////////////////////////
// getBoolInput
bool OsInputDeviceGameControllerSDL::getBoolInput( BcU32 ID ) const
{
	return getFloatInput( ID ) > 0.5f;
}

//////////////////////////////////////////////////////////////////////////
// getIntInput
BcS64 OsInputDeviceGameControllerSDL::getIntInput( BcU32 ID ) const
{
	return static_cast< BcS64 >( getFloatInput( ID ) );
}

//////////////////////////////////////////////////////////////////////////
// getTextInput
const char* OsInputDeviceGameControllerSDL::getTextInput( BcU32 ID ) const
{
	return "";
}

//////////////////////////////////////////////////////////////////////////
// update
void OsInputDeviceGameControllerSDL::update()
{
	auto IsAttached = SDL_GameControllerGetAttached( GameController_ );
	if( IsAttached )
	{
		IsConnected_ = true;

		SDL_HapticUpdateEffect( Haptic_, HapticEffectID_, &HapticEffect_ );
	}
	else
	{
		IsConnected_ = false;
	}
}

//////////////////////////////////////////////////////////////////////////
// setInput
void OsInputDeviceGameControllerSDL::setInput( BcU32 Input, BcU32 Offset, BcF32 Value )
{

}

//////////////////////////////////////////////////////////////////////////
// handleEvent
void OsInputDeviceGameControllerSDL::handleEvent( const union SDL_Event& SDLEvent )
{
	switch( SDLEvent.type )
	{
	case SDL_CONTROLLERAXISMOTION:
		break;
	case SDL_CONTROLLERBUTTONDOWN:
		break;
	case SDL_CONTROLLERBUTTONUP:
		break;
	case SDL_CONTROLLERDEVICEADDED:
		IsConnected_ = true;
		break;
	case SDL_CONTROLLERDEVICEREMOVED:
		IsConnected_ = false;
		break;
	}
}

