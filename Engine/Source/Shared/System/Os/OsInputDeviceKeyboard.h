#pragma once

#include "System/Os/OsInputDevice.h"


//////////////////////////////////////////////////////////////////////////
// OsInputDeviceKeyboard
class OsInputDeviceKeyboard:
	public OsInputDevice
{
public:
	enum
	{
		// Scancode range.
		SCANCODE_BEGIN = 0x0000,
		SCANCODE_END = 0x0200,

		// Keycode range.
		KEYCODE_BEGIN = 0x0200,
		KEYCODE_END = 0x0400,

		// Special.
		TEXT = 0x1000,

		//
		INPUT_MAX
	};

public:
	OsInputDeviceKeyboard();
	virtual ~OsInputDeviceKeyboard();

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
	std::array< BcBool, SCANCODE_END - SCANCODE_BEGIN > ScancodeValues_;
	std::array< BcBool, KEYCODE_END - KEYCODE_BEGIN > KeycodeValues_;
	std::array< char, 1024 > TextBuffer_;
};
