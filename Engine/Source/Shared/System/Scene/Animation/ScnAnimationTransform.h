/**************************************************************************
*
* File:		ScnAnimationTransform.h
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNANIMATIONTRANSFORM_H__
#define __SCNANIMATIONTRANSFORM_H__

#include "Base/BcTypes.h"
#include "Math/MaVec3d.h"
#include "Math/MaQuat.h"
#include "Math/MaMat4d.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// ScnAnimationTransform
class ScnAnimationTransform
{
public:
	ScnAnimationTransform();

	/**
	 * Blend 2 transforms together.
	 */
	void blend( const ScnAnimationTransform& A, const ScnAnimationTransform& B, BcF32 T );

	/**
	 * Add 2 transforms together using a reference.
	 */
	void add( const ScnAnimationTransform& Reference, const ScnAnimationTransform& A, const ScnAnimationTransform& B, BcF32 T );

	/**
	 * Get matrix from this transform.
	 */
	void toMatrix( MaMat4d& Matrix ) const;

	/**
	 * Set from matrix.
	 */
	void fromMatrix( const MaMat4d& Matrix );

public:
	BcAlign( MaQuat R_, 16 );
	BcAlign( MaVec3d S_, 16 );
	BcAlign( MaVec3d T_, 16 );
};


//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline void ScnAnimationTransform::blend( const ScnAnimationTransform& A, const ScnAnimationTransform& B, BcF32 T )
{
	R_.slerp( A.R_, B.R_, T );
	S_.lerp( A.S_, A.S_, T );
	T_.lerp( A.T_, B.T_, T );
}

BcForceInline void ScnAnimationTransform::add( const ScnAnimationTransform& Reference, const ScnAnimationTransform& A, const ScnAnimationTransform& B, BcF32 T )
{
	MaQuat RotQuat;
	RotQuat.slerp( MaQuat(), ( ~Reference.R_ ) * B.R_, T );
	R_ = A.R_ * RotQuat;
	S_ = A.S_ + ( B.S_ - Reference.S_ ) * T;
	T_ = A.T_ + ( B.T_ - Reference.T_ ) * T;
}

#endif
