/**************************************************************************
*
* File:		ScnTypes.cpp
* Author:	Neil Richardson 
* Ver/Date:	24/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnTypes.h"

#include "System/Scene/ScnEntity.h"
#include "System/Content/CsPackage.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnEntitySpawnParams::ScnEntitySpawnParams()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnEntitySpawnParams::ScnEntitySpawnParams( 
		BcName InstanceName, BcName Package, BcName Name, const MaMat4d& Transform, ScnEntity* Parent ):
	InstanceName_( InstanceName ),
	Package_( Package ),
	Name_( Name ),
	Transform_( Transform ),
	Parent_( Parent )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnEntitySpawnParams::ScnEntitySpawnParams( 
		BcName InstanceName, ScnEntity* Template, const MaMat4d& Transform, ScnEntity* Parent ):
	InstanceName_( InstanceName ),
	Package_( Template->getPackage()->getName() ),
	Name_( Template->getName() ),
	Transform_( Transform ),
	Parent_( Parent )
{

}
