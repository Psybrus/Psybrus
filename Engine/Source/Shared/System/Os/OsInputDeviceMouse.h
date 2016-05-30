#pragma once

#include "System/Os/OsInputDevice.h"


//////////////////////////////////////////////////////////////////////////
// OsInputDeviceMouse
class OsInputDeviceMouse:
	public OsInputDevice
{
public:
	enum
	{
		// Axis.
		AXIS_X,
		AXIS_Y,
		AXIS_DX,
		AXIS_DY,
		AXIS_SCROLL_X,
		AXIS_SCROLL_Y,

		// Buttons.
		BUTTON_L,
		BUTTON_R,
		BUTTON_M,
		BUTTON_X1,
		BUTTON_X2,
		BUTTON_WHEEL_LEFT,
		BUTTON_WHEEL_RIGHT,
		BUTTON_WHEEL_UP,
		BUTTON_WHEEL_DOWN,

		//
		INPUT_MAX
	};

public:
	OsInputDeviceMouse();
	virtual ~OsInputDeviceMouse();

	BcName getTypeName() const override;
	bool isConnected() const override;
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
	std::array< BcF32, INPUT_MAX > Values_;

	int ResetWheelTimer_;

};
