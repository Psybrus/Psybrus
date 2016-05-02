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

#include "System/Scene/Rendering/ScnShaderFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnShaderDataAttribute
REFLECTION_DEFINE_DERIVED( ScnShaderDataAttribute );

void ScnShaderDataAttribute::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "IsInstancable_", &ScnShaderDataAttribute::IsInstancable_ ),
	};
		
	ReRegisterClass< ScnShaderDataAttribute >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnShaderDataAttribute::ScnShaderDataAttribute( BcName Name, BcBool IsInstancable ):
	IsInstancable_( IsInstancable )
{
	setName( Name );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnShaderDataAttribute::~ScnShaderDataAttribute()
{
}


//////////////////////////////////////////////////////////////////////////
// ScnShaderViewUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderViewUniformBlockData );

void ScnShaderViewUniformBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "InverseProjectionTransform_", &ScnShaderViewUniformBlockData::InverseProjectionTransform_ ),
		new ReField( "ProjectionTransform_", &ScnShaderViewUniformBlockData::ProjectionTransform_ ),
		new ReField( "InverseViewTransform_", &ScnShaderViewUniformBlockData::InverseViewTransform_ ),
		new ReField( "ViewTransform_", &ScnShaderViewUniformBlockData::ViewTransform_ ),
		new ReField( "InverseClipTransform_", &ScnShaderViewUniformBlockData::InverseClipTransform_ ),
		new ReField( "ClipTransform_", &ScnShaderViewUniformBlockData::ClipTransform_ ),
		new ReField( "ViewTime_", &ScnShaderViewUniformBlockData::ViewTime_ ),
		new ReField( "ViewSize_", &ScnShaderViewUniformBlockData::ViewSize_ ),
		new ReField( "NearFar_", &ScnShaderViewUniformBlockData::NearFar_ ),
	};
		
	auto& Class = ReRegisterClass< ScnShaderViewUniformBlockData >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "View", BcFalse ) );
	Class.setFlags( bcRFF_POD );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderLightUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderLightUniformBlockData );

void ScnShaderLightUniformBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "LightPosition_", &ScnShaderLightUniformBlockData::LightPosition_ ),
		new ReField( "LightDirection_", &ScnShaderLightUniformBlockData::LightDirection_ ),
		new ReField( "LightAmbientColour_", &ScnShaderLightUniformBlockData::LightAmbientColour_ ),
		new ReField( "LightDiffuseColour_", &ScnShaderLightUniformBlockData::LightDiffuseColour_ ),
		new ReField( "LightAttn_", &ScnShaderLightUniformBlockData::LightAttn_ )
	};
		
	auto& Class = ReRegisterClass< ScnShaderLightUniformBlockData >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "Light", BcTrue ) );
	Class.setFlags( bcRFF_POD );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderMaterialUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderMaterialUniformBlockData );

void ScnShaderMaterialUniformBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "MaterialBaseColour_", &ScnShaderMaterialUniformBlockData::MaterialBaseColour_ ),
		new ReField( "MaterialMetallic_", &ScnShaderMaterialUniformBlockData::MaterialMetallic_ ),
		new ReField( "MaterialSpecular_", &ScnShaderMaterialUniformBlockData::MaterialSpecular_ ),
		new ReField( "MaterialRoughness_", &ScnShaderMaterialUniformBlockData::MaterialRoughness_ ),
		new ReField( "MaterialUnused_", &ScnShaderMaterialUniformBlockData::MaterialUnused_ ),
	};
		
	auto& Class = ReRegisterClass< ScnShaderMaterialUniformBlockData >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "Material", BcTrue ) );
	Class.setFlags( bcRFF_POD );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderObjectUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderObjectUniformBlockData );

void ScnShaderObjectUniformBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "WorldTransform_", &ScnShaderObjectUniformBlockData::WorldTransform_ ),
		new ReField( "NormalTransform_", &ScnShaderObjectUniformBlockData::NormalTransform_ ),
	};
		
	auto& Class = ReRegisterClass< ScnShaderObjectUniformBlockData >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "Object", BcTrue ) );
	Class.setFlags( bcRFF_POD );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderBoneUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderBoneUniformBlockData );

void ScnShaderBoneUniformBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "BoneTransform_", &ScnShaderBoneUniformBlockData::BoneTransform_ ),
	};
		
	auto& Class = ReRegisterClass< ScnShaderBoneUniformBlockData >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "Bone", BcFalse ) );
	Class.setFlags( bcRFF_POD );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderAlphaTestUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderAlphaTestUniformBlockData );

void ScnShaderAlphaTestUniformBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "AlphaTestParams_", &ScnShaderAlphaTestUniformBlockData::AlphaTestParams_ ),
	};
		
	auto& Class = ReRegisterClass< ScnShaderAlphaTestUniformBlockData >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "AlphaTest", BcFalse ) );
	Class.setFlags( bcRFF_POD );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderPostProcessConfigData
REFLECTION_DEFINE_BASIC( ScnShaderPostProcessConfigData );

void ScnShaderPostProcessConfigData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "InputDimensions_", &ScnShaderPostProcessConfigData::InputDimensions_ ),
		new ReField( "OutputDimensions_", &ScnShaderPostProcessConfigData::OutputDimensions_ ),
	};
		
	auto& Class = ReRegisterClass< ScnShaderPostProcessConfigData >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "PostProcessConfig", BcFalse ) );
	Class.setFlags( bcRFF_POD );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderPostProcessCopyBlockData
REFLECTION_DEFINE_BASIC( ScnShaderPostProcessCopyBlockData );

void ScnShaderPostProcessCopyBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "ColourTransform_", &ScnShaderPostProcessCopyBlockData::ColourTransform_ ),
	};
		
	auto& Class = ReRegisterClass< ScnShaderPostProcessCopyBlockData >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "PostProcessCopy", BcFalse ) );
	Class.setFlags( bcRFF_POD );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderPostProcessBlurBlockData
REFLECTION_DEFINE_BASIC( ScnShaderPostProcessBlurBlockData );

void ScnShaderPostProcessBlurBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "TextureDimensions_", &ScnShaderPostProcessBlurBlockData::TextureDimensions_ ),
		new ReField( "Radius_", &ScnShaderPostProcessBlurBlockData::Radius_ ),
		new ReField( "BlurUnused_", &ScnShaderPostProcessBlurBlockData::BlurUnused_ ),
	};
		
	auto& Class = ReRegisterClass< ScnShaderPostProcessBlurBlockData >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "PostProcessBlur", BcFalse ) );
	Class.setFlags( bcRFF_POD );
}
