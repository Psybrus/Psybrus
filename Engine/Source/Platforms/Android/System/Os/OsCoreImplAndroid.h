/**************************************************************************
*
* File:		OsCoreImplAndroid.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OsCoreImplAndroid_H__
#define __OsCoreImplAndroid_H__

#include "System/Os/OsCore.h"

//////////////////////////////////////////////////////////////////////////
// OsCoreImplAndroid
class OsCoreImplAndroid:
	public OsCore
{
public:
	OsCoreImplAndroid();
	virtual ~OsCoreImplAndroid();
	
	virtual void 			open();
	virtual void 			update();
	virtual void 			close();

private:
	// Events.
	OsEventCore EventCore_;
	OsEventInputKeyboard EventInputKeyboard_;
	OsEventInputMouse EventInputMouse_;
};

#endif

