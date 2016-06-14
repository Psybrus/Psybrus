/**************************************************************************
*
* File:		OsCoreImplWindows.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Os/OsCoreImplWindows.h"

#include "System/SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// System Creator
SYS_CREATOR( OsCoreImplWindows );

//////////////////////////////////////////////////////////////////////////
// Ctor
OsCoreImplWindows::OsCoreImplWindows()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsCoreImplWindows::~OsCoreImplWindows()
{
	
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void OsCoreImplWindows::open()
{
	// Create default input devices.
	InputKeyboard_.reset( new OsInputDeviceKeyboard() );	
	InputMouse_.reset( new OsInputDeviceMouse() );	
	registerInputDevice( InputKeyboard_.get() );
	registerInputDevice( InputMouse_.get() );

	// Enable XInput.
	XInputEnable( TRUE );

	// Create XInput devices.
	for( size_t Idx = 0; Idx < InputXInput_.size(); ++Idx )
	{
		InputXInput_[ Idx ].reset( new OsInputDeviceXInputWindows( static_cast< BcU32 >( Idx ) ) );
		registerInputDevice( InputXInput_[ Idx ].get() );
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void OsCoreImplWindows::update()
{
	OsCore::update();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void OsCoreImplWindows::close()
{
	unregisterInputDevice( InputKeyboard_.get() );
	unregisterInputDevice( InputMouse_.get() );
	InputKeyboard_.reset();
	InputMouse_.reset();

	for( size_t Idx = 0; Idx < InputXInput_.size(); ++Idx )
	{
		unregisterInputDevice( InputXInput_[ Idx ].get() );
		InputXInput_[ Idx ].reset();
	}
}
