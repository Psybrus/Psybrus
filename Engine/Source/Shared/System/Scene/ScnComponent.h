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
// ScnComponent
class ScnComponent:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnComponent );
	DECLARE_VISITABLE( ScnComponent );

public:
	virtual void						initialise();
	virtual void						initialise( const Json::Value& Object );
	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	BcBool								isAttached() const;
	BcBool								isAttached( ScnEntityWeakRef Parent ) const;
	ScnEntityWeakRef					getParentEntity();
	
	/**
	 * Set the spatial tree node we belong in.
	 */
	void								setSpatialTreeNode( ScnSpatialTreeNode* pNode );

	/**
	 * Get the spatial tree node we are in.
	 */
	ScnSpatialTreeNode*					getSpatialTreeNode();

	/**
	 * Get encompassing AABB for component.
	 */
	virtual BcAABB						getAABB();
	
protected:
	struct THeader
	{
	};
	
	THeader*							pHeader_;

	ScnEntityWeakRef					ParentEntity_;

	ScnSpatialTreeNode*					pSpacialTreeNode_;
};

#endif
