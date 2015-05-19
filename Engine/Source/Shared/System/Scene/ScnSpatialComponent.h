/**************************************************************************
*
* File:		ScnSpatialComponent.h
* Author:	Neil Richardson 
* Ver/Date:	13/01/13
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnSpatialComponent_H__
#define __ScnSpatialComponent_H__

#include "System/Renderer/RsCore.h"
#include "System/Scene/ScnTypes.h"
#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnEntity.h"

//////////////////////////////////////////////////////////////////////////
// ScnSpatialComponent
class ScnSpatialComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnSpatialComponent, ScnComponent );
	DECLARE_VISITABLE( ScnSpatialComponent );

public:

	ScnSpatialComponent();
	virtual ~ScnSpatialComponent();
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );
	void setRenderMask( BcU32 RenderMask );
	const BcU32 getRenderMask() const;

	/**
	 * Set lighting material parameters.
	 */
	void setLightingMaterialParams( class ScnMaterialComponent* MaterialComponent );

	/**
	 * Set the spatial tree node we belong in.
	 */
	void setSpatialTreeNode( ScnSpatialTreeNode* pNode );

	/**
	 * Get the spatial tree node we are in.
	 */
	ScnSpatialTreeNode* getSpatialTreeNode();

	virtual MaAABB getAABB() const;

	BcBool isLit() const;

private:
	static void update( const ScnComponentList& Components );

private:
	ScnSpatialTreeNode* pSpatialTreeNode_;
};

#endif
