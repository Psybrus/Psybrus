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

// TODO: After LD.
//#include "ScnNode.h"
//#include "ScnSpacialTree.h"

//////////////////////////////////////////////////////////////////////////
// ScnCore
class ScnCore:
	public SysSystem,
	public BcGlobal< ScnCore >
{
public:
	ScnCore();
	virtual ~ScnCore();
	
	
	
private:
	
};


#endif


