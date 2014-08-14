/**************************************************************************
*
* File:		ScnShaderFileData.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnShaderFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnShaderViewUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderViewUniformBlockData );

void ScnShaderViewUniformBlockData::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "InverseProjectionTransform_",		&ScnShaderViewUniformBlockData::InverseProjectionTransform_ ),
		ReField( "ProjectionTransform_",			&ScnShaderViewUniformBlockData::ProjectionTransform_ ),
		ReField( "InverseViewTransform_",			&ScnShaderViewUniformBlockData::InverseViewTransform_ ),
		ReField( "ViewTransform_",					&ScnShaderViewUniformBlockData::ViewTransform_ ),
		ReField( "ClipTransform_",					&ScnShaderViewUniformBlockData::ClipTransform_ )
	};
		
	ReRegisterClass< ScnShaderViewUniformBlockData >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderLightUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderLightUniformBlockData );

void ScnShaderLightUniformBlockData::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "LightPosition_",					&ScnShaderLightUniformBlockData::LightPosition_ ),
		ReField( "LightDirection_",					&ScnShaderLightUniformBlockData::LightDirection_ ),
		ReField( "LightAmbientColour_",				&ScnShaderLightUniformBlockData::LightAmbientColour_ ),
		ReField( "LightDiffuseColour_",				&ScnShaderLightUniformBlockData::LightDiffuseColour_ ),
		ReField( "LightAttn_",						&ScnShaderLightUniformBlockData::LightAttn_ )
	};
		
	ReRegisterClass< ScnShaderLightUniformBlockData >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderObjectUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderObjectUniformBlockData );

void ScnShaderObjectUniformBlockData::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "WorldTransform_",					&ScnShaderObjectUniformBlockData::WorldTransform_ ),
	};
		
	ReRegisterClass< ScnShaderObjectUniformBlockData >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderBoneUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderBoneUniformBlockData );

void ScnShaderBoneUniformBlockData::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "BoneTransform_",					&ScnShaderBoneUniformBlockData::BoneTransform_ ),
	};
		
	ReRegisterClass< ScnShaderBoneUniformBlockData >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderAlphaTestUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderAlphaTestUniformBlockData );

void ScnShaderAlphaTestUniformBlockData::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "AlphaTestParams_",				&ScnShaderAlphaTestUniformBlockData::AlphaTestParams_ ),
	};
		
	ReRegisterClass< ScnShaderAlphaTestUniformBlockData >( Fields );
}
