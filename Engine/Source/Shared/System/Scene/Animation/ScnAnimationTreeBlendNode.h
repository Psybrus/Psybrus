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
// ScnAnimationTreeBlendType
enum ScnAnimationTreeBlendType
{
	scnATBT_LERP,
	scnATBT_ADD
};

//////////////////////////////////////////////////////////////////////////
// ScnAnimationTreeBlendNode
class ScnAnimationTreeBlendNode:
	public ScnAnimationTreeNode
{
public:
	BCREFLECTION_DECLARE_DERIVED( ScnAnimationTreeNode, ScnAnimationTreeBlendNode );

public:
	ScnAnimationTreeBlendNode();
	virtual ~ScnAnimationTreeBlendNode();

	virtual void initialise( ScnAnimationPose* pReferencePose );
	virtual void setChildNode( BcU32 Idx, ScnAnimationTreeNode* pNode );
	virtual ScnAnimationTreeNode* getChildNode( BcU32 Idx );
	virtual BcU32 getNoofChildNodes() const;
	virtual void preUpdate( BcF32 Tick );
	virtual void update( BcF32 Tick );
	virtual void postUpdate( BcF32 Tick );
	virtual const ScnAnimationPose& getWorkingPose() const;

	void setBlendType( ScnAnimationTreeBlendType Type );
	ScnAnimationTreeBlendType getBlendType() const;

	void setBlendValue( BcF32 Value );
	BcF32 getBlendValue() const;

private:
	ScnAnimationTreeNode* pNodes_[ 2 ];
	ScnAnimationTreeBlendType BlendType_;
	BcF32 BlendValue_;
};

#endif
