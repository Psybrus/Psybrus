/**************************************************************************
*
* File:		GaTransform.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		Game transform.
*		
*
*
* 
**************************************************************************/

#ifndef __GATRANSFORM_H__
#define __GATRANSFORM_H__

#include "BcQuat.h"
#include "BcMat4d.h"

//////////////////////////////////////////////////////////////////////////
// GaTransform
class GaTransform
{
public:
	GaTransform();
	~GaTransform();

	/**
	*	Concatenate.
	*/
	const GaTransform&		operator * ( const GaTransform& Other );

	/**
	*	Set rotation.
	*/
	void					setRotation( const BcQuat& Rotation );

	/**
	*	Set translation.
	*/
	void					setTranslation( const BcVec3d& Translation );

	/**
	*	Get matrix.
	*/
	const BcMat4d&			getMatrix() const;

private:
	BcQuat					Rotation_;
	BcVec3d					Translation_;

private:
	static GaTransform&		tempTransform();
	static BcMat4d&			tempMatrix();

	//
	static GaTransform		TempTransform_[ 16 ];
	static BcU32			TempTransformIdx_;

	static BcMat4d			TempMatrix_[ 16 ];
	static BcU32			TempMatrixIdx_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline GaTransform::GaTransform():
	Rotation_( 0.0f, 0.0f, 0.0f, 1.0f ),
	Translation_( 0.0f, 0.0f, 0.0f )
{
	
}

inline GaTransform::~GaTransform()
{
	
}

inline const GaTransform& GaTransform::operator * ( const GaTransform& Other )
{
	GaTransform& Result = tempTransform();

	Result.Rotation_ = Rotation_ * Other.Rotation_;
	Result.Translation_ = Translation_ + Other.Translation_;

	// Rotate by the input rotation.
	Other.Rotation_.rotateVector( Result.Translation_ );

	return Result;
}

inline void GaTransform::setRotation( const BcQuat& Rotation )
{
	Rotation_ = Rotation;
}

inline void GaTransform::setTranslation( const BcVec3d& Translation )
{
	Translation_ = Translation;
}

inline const BcMat4d& GaTransform::getMatrix() const
{
	BcMat4d& Result = tempMatrix();

	Rotation_.asMatrix4d( Result );
	Result.row3( BcVec4d( Translation_, 1.0f ) );

	return Result;
}

inline GaTransform& GaTransform::tempTransform()
{
	return TempTransform_[ TempTransformIdx_++ & 0xf ];
}

inline BcMat4d& GaTransform::tempMatrix()
{
	return TempMatrix_[ TempMatrixIdx_++ & 0xf ];
}

#endif
