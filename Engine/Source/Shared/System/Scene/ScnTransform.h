/**************************************************************************
*
* File:		ScnTransform.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		Transform.
*		
*
*
* 
**************************************************************************/

#ifndef __ScnTransform_H__
#define __ScnTransform_H__

#include "BcQuat.h"
#include "BcMat4d.h"
#include "BcVectors.h"

//////////////////////////////////////////////////////////////////////////
// ScnTransform
class ScnTransform
{
public:
	ScnTransform();
	~ScnTransform();

	/**
	*	Concatenate.
	*/
	ScnTransform			operator * ( const ScnTransform& Other );

	/**
	*	Set rotation.
	*/
	void					setRotation( const BcQuat& Rotation );

	/**
	*	Set translation.
	*/
	void					setTranslation( const BcVec3d& Translation );

	/**
	*	Get rotation.
	*/
	const BcQuat&			getRotation() const;

	/**
	*	Get translation.
	*/
	const BcVec3d&			getTranslation() const;

	/**
	*	Set matrix.
	*/
	void					setMatrix( const BcMat4d& InMatrix );

	/**
	*	Get matrix.
	*/
	void					getMatrix( BcMat4d& Result ) const;

	/**
	*	Get inverted matrix.
	*/
	void					getInvertedMatrix( BcMat4d& Result ) const;

private:
	BcQuat					Rotation_;
	BcVec3d					Translation_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline ScnTransform::ScnTransform():
	Rotation_( 0.0f, 0.0f, 0.0f, 1.0f ),
	Translation_( 0.0f, 0.0f, 0.0f )
{
	
}

inline ScnTransform::~ScnTransform()
{
	
}

inline ScnTransform ScnTransform::operator * ( const ScnTransform& Other )
{
	ScnTransform Result;

	Result.Rotation_ = Rotation_ * Other.Rotation_;
	Result.Translation_ = Translation_ + Other.Translation_;

	// Rotate by the input rotation.
	Other.Rotation_.rotateVector( Result.Translation_ );

	return Result;
}

inline void ScnTransform::setRotation( const BcQuat& Rotation )
{
	Rotation_ = Rotation;
}

inline void ScnTransform::setTranslation( const BcVec3d& Translation )
{
	Translation_ = Translation;
}

inline void ScnTransform::setMatrix( const BcMat4d& InMatrix )
{
	Rotation_.fromMatrix4d( InMatrix );
	Translation_ = InMatrix.translation();
}

inline const BcQuat& ScnTransform::getRotation() const
{
	return Rotation_;
}

inline const BcVec3d& ScnTransform::getTranslation() const
{
	return Translation_;
}

inline void ScnTransform::getMatrix( BcMat4d& Result ) const
{
	Rotation_.asMatrix4d( Result );
	Result.row3( BcVec4d( Translation_, 1.0f ) );
}

inline void ScnTransform::getInvertedMatrix( BcMat4d& Result ) const
{
	Rotation_.asMatrix4d( Result );
	Result.row3( BcVec4d( Translation_, 1.0f ) );
	Result.inverse();
}

#endif
