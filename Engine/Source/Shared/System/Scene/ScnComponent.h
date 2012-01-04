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

#include "RsCore.h"
#include "CsResource.h"

#include "ScnTypes.h"
#include "ScnVisitor.h"
#include "ScnSpatialTree.h"

//////////////////////////////////////////////////////////////////////////
// ScnComponent
class ScnComponent:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnComponent );
	DECLARE_VISITABLE( ScnComponent );

public:
	void								initialise();
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
	virtual const BcAABB&				getAABB() const;


protected:
	struct THeader
	{
	};
	
	THeader*							pHeader_;

	ScnEntityWeakRef					ParentEntity_;

	ScnSpatialTreeNode*					pSpacialTreeNode_;
};

#endif
