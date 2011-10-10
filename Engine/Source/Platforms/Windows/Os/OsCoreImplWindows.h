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

#include "OsCore.h"

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
};

#endif

