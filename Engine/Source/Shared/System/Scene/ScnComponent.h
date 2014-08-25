/**************************************************************************
*
* File:		ScnComponent.h
* Author:	Neil Richardson 
* Ver/Date:	26/11/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnComponent_H__
#define __ScnComponent_H__

#include "System/Renderer/RsCore.h"
#include "System/Content/CsResource.h"

#include "System/Scene/ScnTypes.h"
#include "System/Scene/ScnVisitor.h"
#include "System/Scene/ScnSpatialTree.h"

#include <json/json.h>

//////////////////////////////////////////////////////////////////////////
// ScnComponentFlags
enum ScnComponentFlags
{
	scnCF_ATTACHED =					0x00000001,
	scnCF_PENDING_ATTACH =				0x00000002,
	scnCF_PENDING_DETACH =				0x00000004,
	scnCF_DIRTY_ATTACHMENTS =			0x00000008,

	scnCF_WANTS_PREUPDATE =				0x00000100,
	scnCF_WANTS_UPDATE =				0x00000200,
	scnCF_WANTS_POSTUPDATE =			0x00000400,
};

//////////////////////////////////////////////////////////////////////////
// ScnComponentAttribute
class ScnComponentAttribute:
	public ReAttribute
{
public:
	REFLECTION_DECLARE_DERIVED( ScnComponentAttribute, ReAttribute );

public:
	ScnComponentAttribute( BcS32 UpdatePriority = 0 );
	int getUpdatePriority() const;

private:
	BcS32 UpdatePriority_;
};
	
//////////////////////////////////////////////////////////////////////////
// ScnComponent
class ScnComponent:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnComponent, CsResource );

public:
	virtual void						initialise();
	virtual void						initialise( const Json::Value& Object );
	virtual void						create();
	virtual void						destroy();

	virtual void						preUpdate( BcF32 Tick );		// Anything that needs a tick before the game wants this.
	virtual void						update( BcF32 Tick );			// Most will want this tick.
	virtual void						postUpdate( BcF32 Tick );		// Anything that needs a tick after the game wants this.

	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	void								setFlag( ScnComponentFlags Flag );
	void								clearFlag( ScnComponentFlags Flag );
	BcBool								isFlagSet( ScnComponentFlags Flag ) const;

	BcBool								isAttached() const;
	BcBool								isAttached( ScnEntityWeakRef Parent ) const;
	void								setParentEntity( ScnEntityWeakRef Entity );
	ScnEntity*							getParentEntity();
	const ScnEntity*					getParentEntity() const;

	/**
	 * Get full name inc. parents.
	 */
	std::string							getFullName();
	
protected:
	BcU32								Flags_;
	ScnEntityWeakRef					ParentEntity_;
};

#endif
