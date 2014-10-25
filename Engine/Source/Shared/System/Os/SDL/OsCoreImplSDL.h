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
	OsEventCore				EventCore_;
	OsEventInputKeyboard	EventInputKeyboard_;
	OsEventInputMouse		EventInputMouse_;
};

#endif

