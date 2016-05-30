#include "System/Os/OsInputDeviceMouse.h"

#include "System/Os/OsCore.h"

//////////////////////////////////////////////////////////////////////////
// Static Data
namespace
{
	BcName InputType_( "MOUSE" );
	std::array< BcName, OsInputDeviceMouse::INPUT_MAX > InputNames_ =
	{
		"AXIS_X",
		"AXIS_Y",
		"AXIS_DX",
		"AXIS_DY",
		"AXIS_SCROLL_X",
		"AXIS_SCROLL_Y",
		"BUTTON_L",
		"BUTTON_R",
		"BUTTON_M",
		"BUTTON_X1",
		"BUTTON_X2",
		"BUTTON_WHEEL_UP",
		"BUTTON_WHEEL_DOWN",
		"BUTTON_WHEEL_LEFT",
		"BUTTON_WHEEL_RIGHT",
	};
}

//////////////////////////////////////////////////////////////////////////
// Ctor
OsInputDeviceMouse::OsInputDeviceMouse()
{
	memset( Values_.data(), 0, sizeof( BcF32 ) * Values_.size() );
	ResetWheelTimer_ = 0;

	// Register for all mouse events.
	// NOTE: If events are deprecated, then this should be too.
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, this, 
		[ this ]( EvtID ID, const EvtBaseEvent& InEvent ) -> eEvtReturn
		{
			const auto& Event = InEvent.get< OsEventInputMouse >();
			setInput( AXIS_X, 0, static_cast< BcF32 >( Event.MouseX_ ) );
			setInput( AXIS_Y, 0, static_cast< BcF32 >( Event.MouseY_ ) );
			setInput( AXIS_DX, 0, static_cast< BcF32 >( Event.MouseDX_ ) );
			setInput( AXIS_DY, 0, static_cast< BcF32 >( Event.MouseDY_ ) );
			setInput( BUTTON_L, Event.ButtonCode_, 1.0f );
			return evtRET_PASS;
		} );

	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, this,
		[ this ]( EvtID ID, const EvtBaseEvent& InEvent ) -> eEvtReturn
		{
			const auto& Event = InEvent.get< OsEventInputMouse >();
			setInput( AXIS_X, 0, static_cast< BcF32 >( Event.MouseX_ ) );
			setInput( AXIS_Y, 0, static_cast< BcF32 >( Event.MouseY_ ) );
			setInput( AXIS_DX, 0, static_cast< BcF32 >( Event.MouseDX_ ) );
			setInput( AXIS_DY, 0, static_cast< BcF32 >( Event.MouseDY_ ) );
			setInput( BUTTON_L, Event.ButtonCode_, 0.0f );
			return evtRET_PASS;
		} );

	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, this,
		[ this ]( EvtID ID, const EvtBaseEvent& InEvent ) -> eEvtReturn
		{
			const auto& Event = InEvent.get< OsEventInputMouse >();
			setInput( AXIS_X, 0, static_cast< BcF32 >( Event.MouseX_ ) );
			setInput( AXIS_Y, 0, static_cast< BcF32 >( Event.MouseY_ ) );
			setInput( AXIS_DX, 0, static_cast< BcF32 >( Event.MouseDX_ ) );
			setInput( AXIS_DY, 0, static_cast< BcF32 >( Event.MouseDY_ ) );
			setInput( BUTTON_L, Event.ButtonCode_, 0.0f );
			return evtRET_PASS;
		} );

	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEWHEEL, this,
		[ this ]( EvtID ID, const EvtBaseEvent& InEvent ) -> eEvtReturn
		{
			const auto& Event = InEvent.get< OsEventInputMouseWheel >();
			setInput( AXIS_SCROLL_X, 0, static_cast< BcF32 >( Event.ScrollX_ ) );
			setInput( AXIS_SCROLL_Y, 0, static_cast< BcF32 >( Event.ScrollY_ ) );

			setInput( BUTTON_WHEEL_LEFT, 0, Event.ScrollX_ < 0.0f ? 1.0f : 0.0f );
			setInput( BUTTON_WHEEL_RIGHT, 0, Event.ScrollX_ > 0.0f ? 1.0f : 0.0f );
			setInput( BUTTON_WHEEL_UP, 0, Event.ScrollY_ < 0.0f ? 1.0f : 0.0f );
			setInput( BUTTON_WHEEL_DOWN, 0, Event.ScrollY_ > 0.0f ? 1.0f : 0.0f );
			return evtRET_PASS;
		} );	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsInputDeviceMouse::~OsInputDeviceMouse()
{
}

//////////////////////////////////////////////////////////////////////////
// getTypeName
BcName OsInputDeviceMouse::getTypeName() const
{
	return InputType_;
}

//////////////////////////////////////////////////////////////////////////
// getPlayerID
BcU32 OsInputDeviceMouse::getPlayerID() const
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// getNoofInputs
BcU32 OsInputDeviceMouse::getNoofInputs() const
{
	return INPUT_MAX;
}

//////////////////////////////////////////////////////////////////////////
// getInputName
BcName OsInputDeviceMouse::getInputName( BcU32 ID ) const
{
	if( ID < INPUT_MAX )
	{
		return InputNames_[ ID ];
	}
	return BcName::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// getInputID
BcU32 OsInputDeviceMouse::getInputID( BcName Name ) const
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
BcF32 OsInputDeviceMouse::getFloatInput( BcU32 ID ) const
{
	if( ID < Values_.size() )
	{
		return Values_[ ID ];
	}
	return 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// getVec2Input
MaVec2d OsInputDeviceMouse::getVec2Input( BcU32 ID ) const
{
	return MaVec2d( getFloatInput( ID ), getFloatInput( ID + 1 ) );
}

//////////////////////////////////////////////////////////////////////////
// getVec3Input
MaVec3d OsInputDeviceMouse::getVec3Input( BcU32 ID ) const
{
	return MaVec3d( getFloatInput( ID ), getFloatInput( ID + 1 ), getFloatInput( ID + 2 ) );
}

//////////////////////////////////////////////////////////////////////////
// getBoolInput
bool OsInputDeviceMouse::getBoolInput( BcU32 ID ) const
{
	return getFloatInput( ID ) > 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// getIntInput
BcS64 OsInputDeviceMouse::getIntInput( BcU32 ID ) const
{
	return static_cast< BcS64 >( getFloatInput( ID ) );
}

//////////////////////////////////////////////////////////////////////////
// getTextInput
const char* OsInputDeviceMouse::getTextInput( BcU32 ID ) const
{
	return "";
}

//////////////////////////////////////////////////////////////////////////
// update
void OsInputDeviceMouse::update()
{
	// Reset scroll. Events will be dispatched after this call.
	setInput( AXIS_SCROLL_X, 0, 0.0f );
	setInput( AXIS_SCROLL_Y, 0, 0.0f );
	setInput( BUTTON_WHEEL_LEFT, 0, 0.0f );
	setInput( BUTTON_WHEEL_RIGHT, 0, 0.0f );
	setInput( BUTTON_WHEEL_UP, 0, 0.0f );
	setInput( BUTTON_WHEEL_DOWN, 0, 0.0f );
}

//////////////////////////////////////////////////////////////////////////
// setInput
void OsInputDeviceMouse::setInput( BcU32 Input, BcU32 Offset, BcF32 Value )
{
	Values_[ (size_t)Input + Offset] = Value;
}
