/**************************************************************************
*
* File:		ScnAnimationFileData.h
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNANIMATIONFILEDATA_H__
#define __SCNANIMATIONFILEDATA_H__

#include "Base/BcHalf.h"
#include "System/Scene/Animation/ScnAnimationTransform.h"

//////////////////////////////////////////////////////////////////////////
// ScnAnimationFlags
enum ScnAnimationFlags
{
	scnAF_DEFAULT =					0x00000000
};

enum ScnAnimationPacking
{
	scnAP_R16S32T32 = 0,
	scnAP_R16S16T16,
	scnAP_R16T32,
	scnAP_R16T16,

	scnAP_MAX,
	scnAP_FORCE_DWORD = 0xffffffff
};

//////////////////////////////////////////////////////////////////////////
// ScnAnimationHeader
struct ScnAnimationHeader
{
	BcU32							NoofNodes_;
	BcU32							NoofPoses_;
	BcU32							Flags_;
	ScnAnimationPacking				Packing_;
};

//////////////////////////////////////////////////////////////////////////
// ScnAnimationNodeFileData
struct ScnAnimationNodeFileData
{
	BcName							Name_;
};

//////////////////////////////////////////////////////////////////////////
// ScnAnimationPoseFileData
struct ScnAnimationPoseFileData
{
	BcF32							Time_;
	BcU32							CRC_;
	BcU32							KeyDataOffset_;
	BcU32							KeyDataSize_;
};

//////////////////////////////////////////////////////////////////////////
// ScnAnimationTransformKey_R16S32T32
struct ScnAnimationTransformKey_R16S32T32
{
	BcS16							RX_;
	BcS16							RY_;
	BcS16							RZ_;
	BcS16							RW_;
	BcF32							SX_;
	BcF32							SY_;
	BcF32							SZ_;
	BcF32							TX_;
	BcF32							TY_;
	BcF32							TZ_;

	BcForceInline void pack( const BcQuat& R, const BcVec3d& S, const BcVec3d& T )
	{
		RX_ = static_cast< BcS16 >( R.x() * 32767.0f );
		RY_ = static_cast< BcS16 >( R.y() * 32767.0f );
		RZ_ = static_cast< BcS16 >( R.z() * 32767.0f );
		RW_ = static_cast< BcS16 >( R.w() * 32767.0f );
		SX_ = S.x();
		SY_ = S.y();
		SZ_ = S.z();
		TX_ = T.x();
		TY_ = T.y();
		TZ_ = T.z();
	}

	BcForceInline void unpack( BcQuat& R, BcVec3d& S, BcVec3d& T ) const
	{
		R.x( static_cast< BcF32 >( RX_ ) / 32767.0f );
		R.y( static_cast< BcF32 >( RY_ ) / 32767.0f );
		R.z( static_cast< BcF32 >( RZ_ ) / 32767.0f );
		R.w( static_cast< BcF32 >( RW_ ) / 32767.0f );
		S.set( SX_, SY_, SZ_ );
		T.set( TX_, TY_, TZ_ );
	}
};

//////////////////////////////////////////////////////////////////////////
// ScnAnimationTransformKey_R16S16T16
struct ScnAnimationTransformKey_R16S16T16
{
	BcS16							RX_;
	BcS16							RY_;
	BcS16							RZ_;
	BcS16							RW_;
	BcU16							SX_;
	BcU16							SY_;
	BcU16							SZ_;
	BcU16							TX_;
	BcU16							TY_;
	BcU16							TZ_;

	BcForceInline void pack( const BcQuat& R, const BcVec3d& S, const BcVec3d& T )
	{
		RX_ = static_cast< BcS16 >( R.x() * 32767.0f );
		RY_ = static_cast< BcS16 >( R.y() * 32767.0f );
		RZ_ = static_cast< BcS16 >( R.z() * 32767.0f );
		RW_ = static_cast< BcS16 >( R.w() * 32767.0f );
		SX_ = BcF32ToHalf( S.x() );
		SY_ = BcF32ToHalf( S.y() );
		SZ_ = BcF32ToHalf( S.z() );
		TX_ = BcF32ToHalf( T.x() );
		TY_ = BcF32ToHalf( T.y() );
		TZ_ = BcF32ToHalf( T.z() );
	}

	BcForceInline void unpack( BcQuat& R, BcVec3d& S, BcVec3d& T ) const
	{
		R.x( static_cast< BcF32 >( RX_ ) / 32767.0f );
		R.y( static_cast< BcF32 >( RY_ ) / 32767.0f );
		R.z( static_cast< BcF32 >( RZ_ ) / 32767.0f );
		R.w( static_cast< BcF32 >( RW_ ) / 32767.0f );
		S.set( BcHalfToF32( SX_ ), BcHalfToF32( SY_ ), BcHalfToF32( SZ_ ) );
		T.set( BcHalfToF32( TX_ ), BcHalfToF32( TY_ ), BcHalfToF32( TZ_ ) );
	}
};

//////////////////////////////////////////////////////////////////////////
// ScnAnimationTransformKey_R16T32
struct ScnAnimationTransformKey_R16T32
{
	BcS16							RX_;
	BcS16							RY_;
	BcS16							RZ_;
	BcS16							RW_;
	BcF32							TX_;
	BcF32							TY_;
	BcF32							TZ_;

	BcForceInline void pack( const BcQuat& R, const BcVec3d& S, const BcVec3d& T )
	{
		RX_ = static_cast< BcS16 >( R.x() * 32767.0f );
		RY_ = static_cast< BcS16 >( R.y() * 32767.0f );
		RZ_ = static_cast< BcS16 >( R.z() * 32767.0f );
		RW_ = static_cast< BcS16 >( R.w() * 32767.0f );
		TX_ = T.x();
		TY_ = T.y();
		TZ_ = T.z();
	}

	BcForceInline void unpack( BcQuat& R, BcVec3d& S, BcVec3d& T ) const
	{
		R.x( static_cast< BcF32 >( RX_ ) / 32767.0f );
		R.y( static_cast< BcF32 >( RY_ ) / 32767.0f );
		R.z( static_cast< BcF32 >( RZ_ ) / 32767.0f );
		R.w( static_cast< BcF32 >( RW_ ) / 32767.0f );
		S.set( 1.0f, 1.0f, 1.0f );
		T.set( TX_, TY_, TZ_ );
	}
};

//////////////////////////////////////////////////////////////////////////
// ScnAnimationTransformKey_R16T16
struct ScnAnimationTransformKey_R16T16
{
	BcS16							RX_;
	BcS16							RY_;
	BcS16							RZ_;
	BcS16							RW_;
	BcU16							TX_;
	BcU16							TY_;
	BcU16							TZ_;

	BcForceInline void pack( const BcQuat& R, const BcVec3d& S, const BcVec3d& T )
	{
		RX_ = static_cast< BcS16 >( R.x() * 32767.0f );
		RY_ = static_cast< BcS16 >( R.y() * 32767.0f );
		RZ_ = static_cast< BcS16 >( R.z() * 32767.0f );
		RW_ = static_cast< BcS16 >( R.w() * 32767.0f );
		TX_ = BcF32ToHalf( T.x() );
		TY_ = BcF32ToHalf( T.y() );
		TZ_ = BcF32ToHalf( T.z() );
	}

	BcForceInline void unpack( BcQuat& R, BcVec3d& S, BcVec3d& T ) const
	{
		R.x( static_cast< BcF32 >( RX_ ) / 32767.0f );
		R.y( static_cast< BcF32 >( RY_ ) / 32767.0f );
		R.z( static_cast< BcF32 >( RZ_ ) / 32767.0f );
		R.w( static_cast< BcF32 >( RW_ ) / 32767.0f );
		S.set( 1.0f, 1.0f, 1.0f );
		T.set( BcHalfToF32( TX_ ), BcHalfToF32( TY_ ), BcHalfToF32( TZ_ ) );
	}
};

#endif
