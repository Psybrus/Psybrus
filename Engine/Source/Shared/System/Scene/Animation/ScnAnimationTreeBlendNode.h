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
	REFLECTION_DECLARE_DERIVED( ScnAnimationTreeBlendNode, ScnAnimationTreeNode );

public:
	ScnAnimationTreeBlendNode();
	virtual ~ScnAnimationTreeBlendNode();

	virtual void initialise( 
		ScnAnimationPose* pReferencePose,
		ScnAnimationNodeFileData* pNodeFileData ) override;
	virtual void setChildNode( BcU32 Idx, ScnAnimationTreeNode* pNode ) override;
	virtual ScnAnimationTreeNode* getChildNode( BcU32 Idx ) override;
	virtual BcU32 getNoofChildNodes() const override;
	virtual void preUpdate( BcF32 Tick ) override;
	virtual void update( BcF32 Tick ) override;
	virtual void postUpdate( BcF32 Tick ) override;
	virtual const ScnAnimationPose& getWorkingPose() const override;

	void setBlendType( ScnAnimationTreeBlendType Type );
	ScnAnimationTreeBlendType getBlendType() const;

	void setBlendValue( BcF32 Value );
	BcF32 getBlendValue() const;

private:
	std::array< ScnAnimationTreeNode*, 2 > Children_;
	ScnAnimationTreeBlendType BlendType_;
	BcF32 BlendValue_;
};

#endif
