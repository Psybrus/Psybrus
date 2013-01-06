/**************************************************************************
*
* File:		ScnAnimationPose.cpp
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Animation/ScnAnimationPose.h"

#include "Base/BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// Reflection.
BCREFLECTION_DEFINE_BASE( ScnAnimationPose );

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationPose::ScnAnimationPose( BcU32 NoofTransforms ):
	NoofTransforms_( NoofTransforms )
{
	pTransforms_ = new ScnAnimationTransform[ NoofTransforms_ ];
}

//////////////////////////////////////////////////////////////////////////
// Copy Ctor
ScnAnimationPose::ScnAnimationPose( const ScnAnimationPose& Pose )
{
	(*this) = Pose;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnAnimationPose::~ScnAnimationPose()
{
	delete [] pTransforms_;
	pTransforms_ = NULL;
	NoofTransforms_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// Assignment
ScnAnimationPose& ScnAnimationPose::operator = ( const ScnAnimationPose& Pose )
{
	NoofTransforms_ = Pose.NoofTransforms_;
	pTransforms_ = new ScnAnimationTransform[ NoofTransforms_ ];
	BcMemCopy( pTransforms_, Pose.pTransforms_, sizeof( ScnAnimationTransform ) * NoofTransforms_ );
	return (*this);
}

//////////////////////////////////////////////////////////////////////////
// blend
void ScnAnimationPose::blend( const ScnAnimationPose& A, const ScnAnimationPose& B, BcF32 T )
{
	BcAssert( A.NoofTransforms_ == NoofTransforms_ );
	BcAssert( B.NoofTransforms_ == NoofTransforms_ );

	for( BcU32 Idx = 0; Idx < NoofTransforms_; ++Idx )
	{
		pTransforms_[ Idx ].blend( A.pTransforms_[ Idx ], B.pTransforms_[ Idx ], T );
	}
}

//////////////////////////////////////////////////////////////////////////
// add
void ScnAnimationPose::add( const ScnAnimationPose& Reference, const ScnAnimationPose& A, const ScnAnimationPose& B, BcF32 T )
{
	BcAssert( A.NoofTransforms_ == NoofTransforms_ );
	BcAssert( B.NoofTransforms_ == NoofTransforms_ );

	for( BcU32 Idx = 0; Idx < NoofTransforms_; ++Idx )
	{
		pTransforms_[ Idx ].add( Reference.pTransforms_[ Idx ], A.pTransforms_[ Idx ], B.pTransforms_[ Idx ], T );
	}
}

//////////////////////////////////////////////////////////////////////////
// normalise
void ScnAnimationPose::normalise()
{
	for( BcU32 Idx = 0; Idx < NoofTransforms_; ++Idx )
	{
		pTransforms_[ Idx ].R_.normalise();
	}
}
