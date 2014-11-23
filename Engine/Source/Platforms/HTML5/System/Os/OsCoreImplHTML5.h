/**************************************************************************
*
* File:		OsCoreImplHTML5.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OsCoreImplHTML5_H__
#define __OsCoreImplHTML5_H__

#include "System/Os/OsCore.h"
#include "System/Os/OsHTML5.h"

//////////////////////////////////////////////////////////////////////////
// OsCoreImplHTML5
class OsCoreImplHTML5:
	public OsCore
{
public:
	OsCoreImplHTML5();
	virtual ~OsCoreImplHTML5();
	
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

