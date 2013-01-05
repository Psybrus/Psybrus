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
#include "Base/BcVectors.h"
#include "Base/BcQuat.h"
#include "Base/BcMat4d.h"

//////////////////////////////////////////////////////////////////////////
// ScnAnimationTransform
class ScnAnimationTransform
{
public:
	ScnAnimationTransform();

	/**
	 * Blend 2 transforms together.
	 */
	void						blend( const ScnAnimationTransform& A, const ScnAnimationTransform& B, BcF32 T );

	/**
	 * Add 2 transforms together using a reference.
	 */
	void						add( const ScnAnimationTransform& Reference, const ScnAnimationTransform& A, const ScnAnimationTransform& B, BcF32 T );

	/**
	 * Get matrix from this transform.
	 */
	void						toMatrix( BcMat4d& Matrix ) const;

	/**
	 * Set from matrix.
	 */
	void						fromMatrix( const BcMat4d& Matrix );

public:
	BcAlign( BcQuat 			R_, 16 );
	BcAlign( BcVec3d 			S_, 16 );
	BcAlign( BcVec3d 			T_, 16 );
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
	BcQuat RotQuat;
	RotQuat.slerp( BcQuat(), ( ~Reference.R_ ) * B.R_, T );
	R_ = A.R_ * RotQuat;
	S_ = A.S_ + ( B.S_ - Reference.S_ ) * T;
	T_ = A.T_ + ( B.T_ - Reference.T_ ) * T;
}

#endif
