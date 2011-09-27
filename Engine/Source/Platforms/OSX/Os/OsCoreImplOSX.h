/**************************************************************************
*
* File:		OsCoreImplOSX.h
* Author:	Neil Richardson 
* Ver/Date:	25/09/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OsCoreImplOSX_H__
#define __OsCoreImplOSX_H__

#include "OsCore.h"

//////////////////////////////////////////////////////////////////////////
// OsCoreImplOSX
class OsCoreImplOSX:
	public OsCore
{
public:
	OsCoreImplOSX();
	virtual ~OsCoreImplOSX();
	
	virtual void 			open();
	virtual void 			update();
	virtual void 			close();
private:
	void					processInputEvents();

private:
	OsEventCore				EventCore_;
	OsEventInputKeyboard	EventInputKeyboard_;
	OsEventInputMouse		EventInputMouse_;

};


#endif


