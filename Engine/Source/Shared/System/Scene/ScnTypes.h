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

#include "Base/BcTypes.h"
#include "Base/BcName.h"
#include "Base/BcMat4d.h"

#include "System/Content/CsResourceRef.h"

//////////////////////////////////////////////////////////////////////////
// ScnRect
struct ScnRect
{
	BcF32 X_, Y_;
	BcF32 W_, H_;
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
// ScnRenderableComponentRef
typedef CsResourceRef< class ScnRenderableComponent > ScnRenderableComponentRef;
typedef CsResourceRef< class ScnRenderableComponent, true > ScnRenderableComponentWeakRef;
typedef std::vector< ScnRenderableComponentRef > ScnRenderableComponentList;
typedef ScnRenderableComponentList::iterator ScnRenderableComponentListIterator;
typedef ScnRenderableComponentList::const_iterator ScnRenderableComponentListConstIterator;
typedef std::map< std::string, ScnRenderableComponentRef > ScnRenderableComponentMap;
typedef ScnRenderableComponentMap::iterator ScnRenderableComponentMapIterator;
typedef ScnRenderableComponentMap::const_iterator ScnRenderableComponentMapConstIterator;

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

//////////////////////////////////////////////////////////////////////////
// ScnEntitySpawnParams
struct ScnEntitySpawnParams
{
	BcName			Package_;
	BcName			Name_;
	BcName			InstanceName_;
	BcMat4d			Transform_;
	ScnEntityRef	Parent_;
};

#endif


