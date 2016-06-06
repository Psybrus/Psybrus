#include "System/Os/OsInputDeviceKeyboard.h"

#include "System/Os/OsCore.h"

//////////////////////////////////////////////////////////////////////////
// Static Data
namespace
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
OsInputDeviceKeyboard::OsInputDeviceKeyboard()
{
	memset( ScancodeValues_.data(), 0, ScancodeValues_.size() * sizeof( BcBool ) );
	memset( KeycodeValues_.data(), 0, KeycodeValues_.size() * sizeof( BcBool )  );
	memset( TextBuffer_.data(), 0, TextBuffer_.size() );

	// Register for all mouse events.
	// NOTE: If events are deprecated, then this should be too.
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, this, 
		[ this ]( EvtID ID, const EvtBaseEvent& InEvent ) -> eEvtReturn
		{
			const auto& Event = InEvent.get< OsEventInputKeyboard >();
			if( Event.ScanCode_ >= 0 && Event.ScanCode_ < ( SCANCODE_END - SCANCODE_BEGIN ) )
			{
				ScancodeValues_[ Event.ScanCode_ ] = BcTrue;
			}
			if( Event.KeyCode_ >= 0 && Event.KeyCode_ < ( KEYCODE_END - KEYCODE_BEGIN ) )
			{
				KeycodeValues_[ Event.KeyCode_ ] = BcTrue;
			}			
			return evtRET_PASS;
		} );

	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP, this,
		[ this ]( EvtID ID, const EvtBaseEvent& InEvent ) -> eEvtReturn
		{
			const auto& Event = InEvent.get< OsEventInputKeyboard >();
			if( Event.ScanCode_ >= 0 && Event.ScanCode_ < 0x1ff )
			{
				ScancodeValues_[ Event.ScanCode_ ] = BcFalse;
			}
			if( Event.KeyCode_ >= 0 && Event.KeyCode_ < 0x1ff )
			{
				KeycodeValues_[ Event.KeyCode_ ] = BcFalse;
			}			
			return evtRET_PASS;
		} );

	OsCore::pImpl()->subscribe( osEVT_INPUT_TEXT, this,
		[ this ]( EvtID ID, const EvtBaseEvent& InEvent ) -> eEvtReturn
		{
			const auto& Event = InEvent.get< OsEventInputText >();
			auto CurrTextLen = strlen( TextBuffer_.data() );
			strncat( TextBuffer_.data(), reinterpret_cast< const char* >( &Event.Text_[0] ), TextBuffer_.size() - CurrTextLen );
			return evtRET_PASS;
		} );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsInputDeviceKeyboard::~OsInputDeviceKeyboard()
{
}

//////////////////////////////////////////////////////////////////////////
// getTypeName
BcName OsInputDeviceKeyboard::getTypeName() const
{
	static BcName InputType_( "KEYBOARD" );
	return InputType_;
}

//////////////////////////////////////////////////////////////////////////
// isConnected
bool OsInputDeviceKeyboard::isConnected() const
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// getPlayerID
BcU32 OsInputDeviceKeyboard::getPlayerID() const
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// getNoofInputs
BcU32 OsInputDeviceKeyboard::getNoofInputs() const
{
	return INPUT_MAX;
}

//////////////////////////////////////////////////////////////////////////
// getInputName
BcName OsInputDeviceKeyboard::getInputName( BcU32 ID ) const
{
	std::array< char, 16 > OutName;
	if( ID >= SCANCODE_BEGIN && ID < SCANCODE_END )
	{
		auto Code = ID - SCANCODE_BEGIN;
		BcSPrintf( OutName.data(), OutName.size(), "SCANCODE_%u", Code );
	}
	else if( ID >= KEYCODE_BEGIN && ID < KEYCODE_END )
	{
		auto Code = ID - KEYCODE_BEGIN;
		BcSPrintf( OutName.data(), OutName.size(), "KEYCODE_%u", Code );
	}
	else if( ID == TEXT )
	{
		static BcName TextName( "TEXT" );
		return TextName;
	}
	return BcName::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// getInputID
BcU32 OsInputDeviceKeyboard::getInputID( BcName Name ) const
{
	if( Name.getValue() == "SCANCODE" )
	{
		return Name.getID() + SCANCODE_BEGIN;
	}
	else if( Name.getValue() == "KEYCODE" )
	{
		return Name.getID() + KEYCODE_BEGIN;
	}
	else if( Name.getValue() == "TEXT" )
	{
		return TEXT;
	}
	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// getFloatInput
BcF32 OsInputDeviceKeyboard::getFloatInput( BcU32 ID ) const
{
	return getBoolInput( ID ) ? 1.0f : 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// getVec2Input
MaVec2d OsInputDeviceKeyboard::getVec2Input( BcU32 ID ) const
{
	return MaVec2d( getFloatInput( ID ), getFloatInput( ID + 1 ) );
}

//////////////////////////////////////////////////////////////////////////
// getVec3Input
MaVec3d OsInputDeviceKeyboard::getVec3Input( BcU32 ID ) const
{
	return MaVec3d( getFloatInput( ID ), getFloatInput( ID + 1 ), getFloatInput( ID + 2 ) );
}

//////////////////////////////////////////////////////////////////////////
// getBoolInput
bool OsInputDeviceKeyboard::getBoolInput( BcU32 ID ) const
{
	if( ID >= SCANCODE_BEGIN && ID < SCANCODE_END )
	{
		return !!ScancodeValues_[ ID - SCANCODE_BEGIN ];
	}
	else if( ID >= KEYCODE_BEGIN && ID < KEYCODE_END )
	{
		return !!KeycodeValues_[ ID - KEYCODE_BEGIN ];
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// getIntInput
BcS64 OsInputDeviceKeyboard::getIntInput( BcU32 ID ) const
{
	return getBoolInput( ID ) ? 1 : 0;
}

//////////////////////////////////////////////////////////////////////////
// getTextInput
const char* OsInputDeviceKeyboard::getTextInput( BcU32 ID ) const
{
	return TextBuffer_.data();
}

//////////////////////////////////////////////////////////////////////////
// update
void OsInputDeviceKeyboard::update()
{
	TextBuffer_[ 0 ] = '\0';
}

//////////////////////////////////////////////////////////////////////////
// setInput
void OsInputDeviceKeyboard::setInput( BcU32 Input, BcU32 Offset, BcF32 Value )
{

}
