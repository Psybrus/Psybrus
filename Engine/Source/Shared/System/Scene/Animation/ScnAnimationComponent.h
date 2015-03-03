/**************************************************************************
*
* File:		ScnAnimationComponent.h
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNANIMATIONCOMPONENT_H__
#define __SCNANIMATIONCOMPONENT_H__

#include "System/Scene/Rendering/ScnModel.h"
#include "System/Scene/Animation/ScnAnimation.h"
#include "System/Scene/Animation/ScnAnimationTreeNode.h"

//////////////////////////////////////////////////////////////////////////
// ScnAnimationComponentRef
typedef ReObjectRef< class ScnAnimationComponent > ScnAnimationComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnCanvasComponent
class ScnAnimationComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnAnimationComponent, ScnComponent );

	ScnAnimationComponent();
	virtual ~ScnAnimationComponent();

	virtual void initialise( const Json::Value& Object );
	void initialiseNode( ScnAnimationTreeNode* pParentNode, BcU32 ChildIndex, const Json::Value& Object );

	virtual void destroy();
	virtual void preUpdate( BcF32 Tick );
	virtual void update( BcF32 Tick );
	virtual void postUpdate( BcF32 Tick );

	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

	/**
	 * Find a node by type.
	 */
	template< class _Ty >
	_Ty* findNodeByType( const BcName& Name )
	{
		return static_cast< _Ty* >( findNodeRecursively( pRootTreeNode_, Name, _Ty::StaticGetClass() ) );
	}

private:
	void buildReferencePose();
	void applyPose();

	ScnAnimationTreeNode* findNodeRecursively( ScnAnimationTreeNode* pStartNode, const BcName& Name, const ReClass* Class );

private:
	BcName TargetComponentName_;
	ScnModelComponentRef Model_;

	/// Node file data for the model we are building a pose for.
	std::vector< ScnAnimationNodeFileData > ModelNodeFileData_;

	/// Root node in the animation tree.
	ScnAnimationTreeNode* pRootTreeNode_;

	/// Reference pose.
	ScnAnimationPose* pReferencePose_;
};

#endif
