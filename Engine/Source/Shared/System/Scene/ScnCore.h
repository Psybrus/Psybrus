/**************************************************************************
*
* File:		ScnCore.h
* Author:	Neil Richardson 
* Ver/Date:	23/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnCore_H__
#define __ScnCore_H__

#include "BcTypes.h"
#include "BcGlobal.h"

#include "SysSystem.h"

#include "ScnEntity.h"
#include "ScnSpacialTree.h"

//////////////////////////////////////////////////////////////////////////
// ScnCore
class ScnCore:
	public SysSystem,
	public BcGlobal< ScnCore >
{
public:
	ScnCore();
	virtual ~ScnCore();

	virtual void				open();
	virtual void				update();
	virtual void				close();

	void						addEntity( ScnEntityRef Entity );
	void						removeEntity( ScnEntityRef Entity );	
	
private:
	ScnSpacialTree*				pSpacialTree_;
	ScnEntityList				EntityList_;
};


#endif


