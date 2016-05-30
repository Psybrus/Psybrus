/**************************************************************************
*
* File:		OsCoreImplWindows.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OSCOREIMPLWINDOWS_H__
#define __OSCOREIMPLWINDOWS_H__

#include "System/Os/OsCore.h"

#include "System/Os/OsInputDeviceKeyboard.h"
#include "System/Os/OsInputDeviceMouse.h"

//////////////////////////////////////////////////////////////////////////
// OsCoreImplWindows
class OsCoreImplWindows:
	public OsCore
{
public:
	OsCoreImplWindows();
	virtual ~OsCoreImplWindows();
	
	virtual void 			open();
	virtual void 			update();
	virtual void 			close();

private:
	// Events.
	OsEventCore				EventCore_;
	OsEventInputKeyboard	EventInputKeyboard_;
	OsEventInputMouse		EventInputMouse_;

	std::unique_ptr< OsInputDeviceKeyboard > InputKeyboard_;
	std::unique_ptr< OsInputDeviceMouse > InputMouse_;
};

#endif

