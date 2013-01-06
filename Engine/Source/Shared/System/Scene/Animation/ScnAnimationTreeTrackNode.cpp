/**************************************************************************
*
* File:		ScnAnimationTreeTrackNode.cpp
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Animation/ScnAnimationTreeTrackNode.h"

//////////////////////////////////////////////////////////////////////////
// Reflection.
BCREFLECTION_DEFINE_DERIVED( ScnAnimationTreeTrackNode );

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationTreeTrackNode::ScnAnimationTreeTrackNode( const BcName& Name ):
	ScnAnimationTreeNode( Name )
{
	pPoseA_ = NULL;
	pPoseB_ = NULL;
	pPoseFileDataA_ = NULL;
	pPoseFileDataB_ = NULL;
	CurrPoseIndex_ = BcErrorCode;
	Time_ = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnAnimationTreeTrackNode::~ScnAnimationTreeTrackNode()
{
	delete pPoseA_;
	delete pPoseB_;
	pPoseA_ = NULL;
	pPoseB_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// setChildNode
//virtual
void ScnAnimationTreeTrackNode::initialise( ScnAnimationPose* pReferencePose )
{
	ScnAnimationTreeNode::initialise( pReferencePose );

	pPoseA_ = new ScnAnimationPose( *pReferencePose );
	pPoseB_ = new ScnAnimationPose( *pReferencePose );
}

//////////////////////////////////////////////////////////////////////////
// setChildNode
//virtual
void ScnAnimationTreeTrackNode::setChildNode( BcU32 Idx, ScnAnimationTreeNode* pNode )
{
	BcUnusedVar( Idx );
	BcUnusedVar( pNode );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// getChildNode
//virtual
ScnAnimationTreeNode* ScnAnimationTreeTrackNode::getChildNode( BcU32 Idx )
{
	BcUnusedVar( Idx );
	BcBreakpoint;
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getNoofChildNodes
//virtual
BcU32 ScnAnimationTreeTrackNode::getNoofChildNodes() const
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// preUpdate
//virtual
void ScnAnimationTreeTrackNode::preUpdate( BcF32 Tick )
{
	// Do time line updating, and kick off job to unpack the animation.
	decodeFrames();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnAnimationTreeTrackNode::update( BcF32 Tick )
{
	// Interpolate poses.
	interpolatePose();

	// Advance time.
	Time_ += Tick;
}

//////////////////////////////////////////////////////////////////////////
// postUpdate
//virtual
void ScnAnimationTreeTrackNode::postUpdate( BcF32 Tick )
{

}

//////////////////////////////////////////////////////////////////////////
// queueAnimation
void ScnAnimationTreeTrackNode::queueAnimation( ScnAnimation* pAnimation )
{
	AnimationQueue_.push_back( pAnimation );
}

//////////////////////////////////////////////////////////////////////////
// decodeFrames
void ScnAnimationTreeTrackNode::decodeFrames()
{
	if( AnimationQueue_.size() > 0 )
	{
		// Grab current animation.
		ScnAnimation* pCurrAnimation = AnimationQueue_[ 0 ];

		// Wrap time (for advancement and looping)
		if( Time_ > pCurrAnimation->getLength() )
		{
			Time_ -= pCurrAnimation->getLength();
		}
		
		// Find pose index for time from animation.
		BcU32 NewPoseIndex = pCurrAnimation->findPoseIndexAtTime( Time_ );

		// If it doesn't match, we need to decode frames.
		if( CurrPoseIndex_ != NewPoseIndex )
		{
			CurrPoseIndex_ = NewPoseIndex;
			pCurrAnimation->decodePoseAtIndex( CurrPoseIndex_, pPoseA_ );
			pCurrAnimation->decodePoseAtIndex( CurrPoseIndex_ + 1, pPoseB_ );

			pPoseFileDataA_ = pCurrAnimation->findPoseAtIndex( CurrPoseIndex_ );
			pPoseFileDataB_ = pCurrAnimation->findPoseAtIndex( CurrPoseIndex_ + 1 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// interpolatePose
void ScnAnimationTreeTrackNode::interpolatePose()
{
	const BcF32 TimeLength = pPoseFileDataB_->Time_ - pPoseFileDataA_->Time_;
	const BcF32 TimeRelative = Time_ - pPoseFileDataA_->Time_;
	const BcF32 LerpAmount = TimeRelative / TimeLength;
	
	pWorkingPose_->blend( *pPoseA_, *pPoseB_, LerpAmount );
}
