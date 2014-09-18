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
REFLECTION_DEFINE_DERIVED( ScnAnimationTreeTrackNode );

void ScnAnimationTreeTrackNode::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pPoseA_", &ScnAnimationTreeTrackNode::pPoseA_ ),
		new ReField( "pPoseB_", &ScnAnimationTreeTrackNode::pPoseB_ ),
		new ReField( "CurrPoseIndex_", &ScnAnimationTreeTrackNode::CurrPoseIndex_, bcRFF_TRANSIENT ),
		new ReField( "Speed_", &ScnAnimationTreeTrackNode::Speed_ ),
		new ReField( "Time_", &ScnAnimationTreeTrackNode::Time_ ),
		new ReField( "AnimationQueue_", &ScnAnimationTreeTrackNode::AnimationQueue_, bcRFF_TRANSIENT ),
		new ReField( "pPoseFileDataA_", &ScnAnimationTreeTrackNode::pPoseFileDataA_, bcRFF_TRANSIENT ),
		new ReField( "pPoseFileDataB_", &ScnAnimationTreeTrackNode::pPoseFileDataB_, bcRFF_TRANSIENT ),
	};
		
	ReRegisterClass< ScnAnimationTreeTrackNode, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationTreeTrackNode::ScnAnimationTreeTrackNode()
{
	pPoseA_ = nullptr;
	pPoseB_ = nullptr;
	CurrPoseIndex_ = 0;
	Time_ = 0.0f;
	pPoseFileDataA_ = nullptr;
	pPoseFileDataB_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnAnimationTreeTrackNode::~ScnAnimationTreeTrackNode()
{
	delete pPoseA_;
	delete pPoseB_;
	pPoseA_ = nullptr;
	pPoseB_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// setChildNode
//virtual
void ScnAnimationTreeTrackNode::initialise( ScnAnimationPose* pReferencePose )
{
	ScnAnimationTreeNode::initialise( pReferencePose );

	CurrPoseIndex_ = 0;
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
	Time_ += Tick * Speed_;
}

//////////////////////////////////////////////////////////////////////////
// postUpdate
//virtual
void ScnAnimationTreeTrackNode::postUpdate( BcF32 Tick )
{

}

//////////////////////////////////////////////////////////////////////////
// setSpeed
void ScnAnimationTreeTrackNode::setSpeed( BcF32 Speed )
{
	BcAssert( Speed >= 0.0f );
	Speed_ = Speed;
}

//////////////////////////////////////////////////////////////////////////
// queueAnimation
void ScnAnimationTreeTrackNode::queueAnimation( ScnAnimation* pAnimation )
{
	BcAssert( pAnimation != NULL );
	AnimationQueue_.push_back( pAnimation );

	// TODO: Create a map to map any animation by node name onto our reference
	//       pose. Probably want to do this asynchronously on a job.
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
		if( CurrPoseIndex_ != NewPoseIndex ||
			pPoseFileDataA_ == nullptr ||
			pPoseFileDataB_ == nullptr )
		{
			CurrPoseIndex_ = NewPoseIndex;

			// TODO: Pass in a map to decode these correctly into the appropriate transforms.
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
	if( AnimationQueue_.size() > 0 )
	{
		BcAssert( pPoseFileDataA_ != nullptr );
		BcAssert( pPoseFileDataB_ != nullptr );
		const BcF32 TimeLength = pPoseFileDataB_->Time_ - pPoseFileDataA_->Time_;
		const BcF32 TimeRelative = Time_ - pPoseFileDataA_->Time_;
		const BcF32 LerpAmount = TimeRelative / TimeLength;
	
		pWorkingPose_->blend( *pPoseA_, *pPoseB_, LerpAmount );
	}
}
