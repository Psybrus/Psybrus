/**************************************************************************
*
* File:		ScnAnimationPose.h
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNANIMATIONPOSE_H__
#define __SCNANIMATIONPOSE_H__

#include "System/Scene/Animation/ScnAnimationTransform.h"

//////////////////////////////////////////////////////////////////////////
// ScnAnimationPose
class ScnAnimationPose
{
public:
	REFLECTION_DECLARE_BASE( ScnAnimationPose );

public:
	ScnAnimationPose();
	ScnAnimationPose( BcU32 NoofTransforms );
	ScnAnimationPose( const ScnAnimationPose& Pose );
	virtual ~ScnAnimationPose();
	ScnAnimationPose& operator = ( const ScnAnimationPose& Pose );

	/**
	 * Set transform.
	 */
	void setTransform( BcU32 Idx, const ScnAnimationTransform& Transform );

	/**
	 * Get transform.
	 */
	const ScnAnimationTransform& getTransform( BcU32 Idx ) const;

	/**
	 * Blend 2 poses together.
	 */
	void blend( const ScnAnimationPose& A, const ScnAnimationPose& B, BcF32 T );

	/**
	 * Add 2 poses together using a reference pose. A + ( B - Reference )
	 */
	void add( const ScnAnimationPose& Reference, const ScnAnimationPose& A, const ScnAnimationPose& B, BcF32 T );

	/**
	 * Normalise.
	 */
	void normalise();

	/**
	 * Get number of nodes.
	 */
	BcU32 getNoofNodes() const;

private:
	std::vector< ScnAnimationTransform > Transforms_;
};

BcForceInline void ScnAnimationPose::setTransform( BcU32 Idx, const ScnAnimationTransform& Transform )
{
	BcAssert( Idx < Transforms_.size() );
	Transforms_[ Idx ] = Transform;
}

BcForceInline const ScnAnimationTransform& ScnAnimationPose::getTransform( BcU32 Idx ) const
{
	BcAssert( Idx < Transforms_.size() );
	return Transforms_[ Idx ];
}

#endif
