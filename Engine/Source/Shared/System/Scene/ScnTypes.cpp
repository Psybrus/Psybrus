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
ScnEntitySpawnParams::ScnEntitySpawnParams():
	InstanceName_( BcName::INVALID ),
	Package_( BcName::INVALID ),
	Name_( BcName::INVALID ),
	Template_( nullptr ),
	Transform_( MaMat4d() ),
	Parent_()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnEntitySpawnParams::ScnEntitySpawnParams( 
		BcName InstanceName, BcName Package, BcName Name, const MaMat4d& Transform, ScnEntity* Parent ):
	InstanceName_( InstanceName ),
	Package_( Package ),
	Name_( Name ),
	Template_( nullptr ),
	Transform_( Transform ),
	Parent_( Parent )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnEntitySpawnParams::ScnEntitySpawnParams( 
		BcName InstanceName, ScnEntity* Template, const MaMat4d& Transform, ScnEntity* Parent ):
	InstanceName_( InstanceName ),
	Package_(),
	Name_(),
	Template_( Template ),
	Transform_( Transform ),
	Parent_( Parent )
{

}
