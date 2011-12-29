/**************************************************************************
*
* File:		ScnTypes.h
* Author:	Neil Richardson 
* Ver/Date:	24/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnTypes_H__
#define __ScnTypes_H__

#include "BcTypes.h"

#include "CsResourceRef.h"

//////////////////////////////////////////////////////////////////////////
// ScnRect
struct ScnRect
{
	BcReal X_, Y_;
	BcReal W_, H_;
};

//////////////////////////////////////////////////////////////////////////
// ScnComponentRef
typedef CsResourceRef< class ScnComponent > ScnComponentRef;
typedef CsResourceRef< class ScnComponent, true > ScnComponentWeakRef;
typedef std::vector< ScnComponentRef > ScnComponentList;
typedef ScnComponentList::iterator ScnComponentListIterator;
typedef ScnComponentList::const_iterator ScnComponentListConstIterator;
typedef std::map< std::string, ScnComponentRef > ScnComponentMap;
typedef ScnComponentMap::iterator ScnComponentMapIterator;
typedef ScnComponentMap::const_iterator ScnComponentMapConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnEntityRef
typedef CsResourceRef< class ScnEntity > ScnEntityRef;
typedef CsResourceRef< class ScnEntity, true > ScnEntityWeakRef;
typedef std::list< ScnEntityRef > ScnEntityList;
typedef ScnEntityList::iterator ScnEntityListIterator;
typedef ScnEntityList::const_iterator ScnEntityListConstIterator;
typedef std::map< std::string, ScnEntityRef > ScnEntityMap;
typedef ScnEntityMap::iterator ScnEntityMapIterator;
typedef ScnEntityMap::const_iterator ScnEntityMapConstIterator;

#endif


