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
#include "Math/MaMat4d.h"

#include "Reflection/ReObjectRef.h"

#include <vector>
#include <map>

// NOTE: Look at breaking this up a little. It's mostly a hack to forward declare
//       everything.

//////////////////////////////////////////////////////////////////////////
// ScnRect
struct ScnRect
{
	BcF32 X_, Y_;
	BcF32 W_, H_;
};

//////////////////////////////////////////////////////////////////////////
// ScnComponentRef
typedef ReObjectRef< class ScnComponent > ScnComponentRef;
typedef ReObjectRef< class ScnComponent, true > ScnComponentWeakRef;
typedef std::vector< ScnComponentRef > ScnComponentList;
typedef ScnComponentList::iterator ScnComponentListIterator;
typedef ScnComponentList::const_iterator ScnComponentListConstIterator;
typedef std::map< std::string, ScnComponentRef > ScnComponentMap;
typedef ScnComponentMap::iterator ScnComponentMapIterator;
typedef ScnComponentMap::const_iterator ScnComponentMapConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnRenderableComponentRef
typedef ReObjectRef< class ScnRenderableComponent > ScnRenderableComponentRef;
typedef ReObjectRef< class ScnRenderableComponent, true > ScnRenderableComponentWeakRef;
typedef std::vector< ScnRenderableComponentRef > ScnRenderableComponentList;
typedef ScnRenderableComponentList::iterator ScnRenderableComponentListIterator;
typedef ScnRenderableComponentList::const_iterator ScnRenderableComponentListConstIterator;
typedef std::map< std::string, ScnRenderableComponentRef > ScnRenderableComponentMap;
typedef ScnRenderableComponentMap::iterator ScnRenderableComponentMapIterator;
typedef ScnRenderableComponentMap::const_iterator ScnRenderableComponentMapConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnEntityRef
typedef ReObjectRef< class ScnEntity > ScnEntityRef;
typedef ReObjectRef< class ScnEntity, true > ScnEntityWeakRef;
typedef std::vector< ScnEntityRef > ScnEntityList;
typedef ScnEntityList::iterator ScnEntityListIterator;
typedef ScnEntityList::const_iterator ScnEntityListConstIterator;
typedef std::map< std::string, ScnEntityRef > ScnEntityMap;
typedef ScnEntityMap::iterator ScnEntityMapIterator;
typedef ScnEntityMap::const_iterator ScnEntityMapConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnEntitySpawnParams
struct ScnEntitySpawnParams
{
	ScnEntitySpawnParams();
	ScnEntitySpawnParams( BcName InstanceName, BcName Package, BcName Name, const MaMat4d& Transform, ScnEntity* Parent );
	ScnEntitySpawnParams( BcName InstanceName, ScnEntity* Template, const MaMat4d& Transform, ScnEntity* Parent );

	BcName InstanceName_;
	BcName Package_;
	BcName Name_;
	MaMat4d Transform_;
	ScnEntityRef Parent_;
	std::function< void( ScnEntity* ) > OnSpawn_;
};

#endif


