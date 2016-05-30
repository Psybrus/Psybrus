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
}
