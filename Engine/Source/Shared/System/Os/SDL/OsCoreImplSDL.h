/**************************************************************************
*
* File:		OsCoreImplSDL.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OsCoreImplSDL_H__
#define __OsCoreImplSDL_H__

#include "System/Os/OsCore.h"
#include "System/Os/SDL/OsSDL.h"
#include "System/Os/OsInputDeviceKeyboard.h"
#include "System/Os/OsInputDeviceMouse.h"

//////////////////////////////////////////////////////////////////////////
// OsCoreImplSDL
class OsCoreImplSDL:
	public OsCore
{
public:
	OsCoreImplSDL();
	virtual ~OsCoreImplSDL();
	
	virtual void 			open();
	virtual void 			update();
	virtual void 			close();

private:
	// Events.
	OsEventCore EventCore_;
	OsEventInputKeyboard EventInputKeyboard_;
	OsEventInputMouse EventInputMouse_;

	std::unique_ptr< OsInputDeviceKeyboard > InputKeyboard_;
	std::unique_ptr< OsInputDeviceMouse > InputMouse_;
	std::vector< std::unique_ptr< class OsInputDeviceGameControllerSDL > > InputGameControllers_;
};

#endif

