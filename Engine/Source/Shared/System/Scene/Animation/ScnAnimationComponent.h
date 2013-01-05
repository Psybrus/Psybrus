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

#include "System/Scene/ScnModel.h"
#include "System/Scene/Animation/ScnAnimation.h"
#include "System/Scene/Animation/ScnAnimationTreeNode.h"

//////////////////////////////////////////////////////////////////////////
// ScnAnimationComponentRef
typedef CsResourceRef< class ScnAnimationComponent > ScnAnimationComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnCanvasComponent
class ScnAnimationComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnAnimationComponent );
	
	virtual void						initialise( const Json::Value& Object );

	virtual void						preUpdate( BcF32 Tick );
	virtual void						update( BcF32 Tick );
	virtual void						postUpdate( BcF32 Tick );

	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

private:
	void								buildReferencePose();
	void								applyPose();

private:
	BcName								TargetComponentName_;
	ScnModelComponentRef				Model_;

	ScnAnimationTreeNode*				pRootTreeNode_;
	ScnAnimationPose*					pReferencePose_;
};

#endif
