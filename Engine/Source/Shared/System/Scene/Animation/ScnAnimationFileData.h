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

#include "System/Scene/Animation/ScnAnimationTransform.h"

//////////////////////////////////////////////////////////////////////////
// ScnAnimationFlags
enum ScnAnimationFlags
{
	scnAF_DEFAULT =					0x00000000
};

//////////////////////////////////////////////////////////////////////////
// ScnAnimationHeader
struct ScnAnimationHeader
{
	BcU32							NoofNodes_;
	BcU32							NoofPoses_;
	BcU32							Flags_;
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
// ScnAnimationTransformKey
struct ScnAnimationTransformKey
{
	BcQuat							R_;
	BcVec3d							S_;
	BcVec3d							T_;

	BcForceInline void pack( const BcQuat& R, const BcVec3d& S, const BcVec3d& T )
	{
		R_ = R;
		S_ = S;
		T_ = T;
	}

	BcForceInline void unpack( BcQuat& R, BcVec3d& S, BcVec3d& T ) const
	{
		R = R_;
		S = S_;
		T = T_;
	}
};

#endif
