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
#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Reflection.
REFLECTION_DEFINE_BASE( ScnAnimationPose );

void ScnAnimationPose::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Transforms_", &ScnAnimationPose::Transforms_ ),
	};
		
	ReRegisterClass< ScnAnimationPose >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationPose::ScnAnimationPose():
	Transforms_()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationPose::ScnAnimationPose( BcU32 NoofTransforms )
{
	Transforms_.resize( NoofTransforms );
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
	Transforms_.clear();
}

//////////////////////////////////////////////////////////////////////////
// Assignment
ScnAnimationPose& ScnAnimationPose::operator = ( const ScnAnimationPose& Pose )
{
	PSY_PROFILER_SECTION( TickRoot, "ScnAnimationPose::operator =" );

	Transforms_ = Pose.Transforms_;
	return (*this);
}

//////////////////////////////////////////////////////////////////////////
// blend
void ScnAnimationPose::blend( const ScnAnimationPose& A, const ScnAnimationPose& B, BcF32 T )
{
	PSY_PROFILER_SECTION( TickRoot, "ScnAnimationPose::blend" );

	BcAssert( A.Transforms_.size() == Transforms_.size() );
	BcAssert( B.Transforms_.size() == Transforms_.size() );

	for( BcU32 Idx = 0; Idx < Transforms_.size(); ++Idx )
	{
		Transforms_[ Idx ].blend( A.Transforms_[ Idx ], B.Transforms_[ Idx ], T );
	}
}

//////////////////////////////////////////////////////////////////////////
// add
void ScnAnimationPose::add( const ScnAnimationPose& Reference, const ScnAnimationPose& A, const ScnAnimationPose& B, BcF32 T )
{
	PSY_PROFILER_SECTION( TickRoot, "ScnAnimationPose::add" );

	BcAssert( Reference.Transforms_.size() == Transforms_.size() );
	BcAssert( A.Transforms_.size() == Transforms_.size() );
	BcAssert( B.Transforms_.size() == Transforms_.size() );

	for( BcU32 Idx = 0; Idx < Transforms_.size(); ++Idx )
	{
		Transforms_[ Idx ].add( Reference.Transforms_[ Idx ], A.Transforms_[ Idx ], B.Transforms_[ Idx ], T );
	}
}

//////////////////////////////////////////////////////////////////////////
// normalise
void ScnAnimationPose::normalise()
{
	PSY_PROFILER_SECTION( TickRoot, "ScnAnimationPose::normalise" );

	for( BcU32 Idx = 0; Idx < Transforms_.size(); ++Idx )
	{
		Transforms_[ Idx ].R_.normalise();
	}
}

//////////////////////////////////////////////////////////////////////////
// getNoofNodes
BcU32 ScnAnimationPose::getNoofNodes() const
{
	return Transforms_.size();
}
