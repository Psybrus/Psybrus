/**************************************************************************
*
* File:		ScnAnimationTreeBlendNode.h
* Author:	Neil Richardson 
* Ver/Date:	06/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNANIMATIONTREEBLENDNODE_H__
#define __SCNANIMATIONTREEBLENDNODE_H__

#include "System/Scene/Animation/ScnAnimationTreeNode.h"
#include "System/Scene/Animation/ScnAnimation.h"

//////////////////////////////////////////////////////////////////////////
// ScnAnimationTreeBlendNode
class ScnAnimationTreeBlendNode:
	public ScnAnimationTreeNode
{
public:
	BCREFLECTION_DECLARE_DERIVED( ScnAnimationTreeNode, ScnAnimationTreeBlendNode );

public:
	ScnAnimationTreeBlendNode( const BcName& Name );
	virtual ~ScnAnimationTreeBlendNode();

	virtual void initialise( ScnAnimationPose* pReferencePose );
	virtual void setChildNode( BcU32 Idx, ScnAnimationTreeNode* pNode );
	virtual ScnAnimationTreeNode* getChildNode( BcU32 Idx );
	virtual BcU32 getNoofChildNodes() const;
	virtual void preUpdate( BcF32 Tick );
	virtual void update( BcF32 Tick );
	virtual void postUpdate( BcF32 Tick );

	void setBlendValue( BcF32 Value );
	BcF32 getBlendValue() const;

private:
	ScnAnimationTreeNode* pNodes_[ 2 ];
	BcF32 BlendValue_;
};

#endif
