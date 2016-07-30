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
		new ReField( "MaterialRoughness_", &ScnShaderMaterialUniformBlockData::MaterialRoughness_ ),
		new ReField( "MaterialUnused0_", &ScnShaderMaterialUniformBlockData::MaterialUnused0_ ),
		new ReField( "MaterialUnused1_", &ScnShaderMaterialUniformBlockData::MaterialUnused1_ ),
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
// ScnShaderDownsampleUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderDownsampleUniformBlockData );

void ScnShaderDownsampleUniformBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "DownsampleSourceMipLevel_", &ScnShaderDownsampleUniformBlockData::DownsampleSourceMipLevel_ ),
		new ReField( "DownsampleUnused_", &ScnShaderDownsampleUniformBlockData::DownsampleUnused_ ),
	};
		
	auto& Class = ReRegisterClass< ScnShaderDownsampleUniformBlockData >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "Downsample", BcFalse ) );
	Class.setFlags( bcRFF_POD );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderBloomUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderBloomUniformBlockData );

void ScnShaderBloomUniformBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "BloomRadius_", &ScnShaderBloomUniformBlockData::BloomRadius_ ),
		new ReField( "BloomPower_", &ScnShaderBloomUniformBlockData::BloomPower_ ),
		new ReField( "BloomThreshold_", &ScnShaderBloomUniformBlockData::BloomThreshold_ ),
		new ReField( "BloomUnused_", &ScnShaderBloomUniformBlockData::BloomUnused_ ),
	};
		
	auto& Class = ReRegisterClass< ScnShaderBloomUniformBlockData >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "Bloom", BcFalse ) );
	Class.setFlags( bcRFF_POD );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderToneMappingUniformBlockData
REFLECTION_DEFINE_BASIC( ScnShaderToneMappingUniformBlockData );

void ScnShaderToneMappingUniformBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "ToneMappingExposure_", &ScnShaderToneMappingUniformBlockData::ToneMappingExposure_ ),
		new ReField( "ToneMappingMiddleGrey_", &ScnShaderToneMappingUniformBlockData::ToneMappingMiddleGrey_ ),
		new ReField( "ToneMappingWhitePoint_", &ScnShaderToneMappingUniformBlockData::ToneMappingWhitePoint_ ),
		new ReField( "ToneMappingA_", &ScnShaderToneMappingUniformBlockData::ToneMappingA_ ),
		new ReField( "ToneMappingB_", &ScnShaderToneMappingUniformBlockData::ToneMappingB_ ),
		new ReField( "ToneMappingC_", &ScnShaderToneMappingUniformBlockData::ToneMappingC_ ),
		new ReField( "ToneMappingD_", &ScnShaderToneMappingUniformBlockData::ToneMappingD_ ),
		new ReField( "ToneMappingE_", &ScnShaderToneMappingUniformBlockData::ToneMappingE_ ),
		new ReField( "ToneMappingF_", &ScnShaderToneMappingUniformBlockData::ToneMappingF_ ),
		new ReField( "ToneMappingLuminanceMin_", &ScnShaderToneMappingUniformBlockData::ToneMappingLuminanceMin_ ),
		new ReField( "ToneMappingLuminanceMax_", &ScnShaderToneMappingUniformBlockData::ToneMappingLuminanceMax_ ),
		new ReField( "ToneMappingLuminanceTransferRate_", &ScnShaderToneMappingUniformBlockData::ToneMappingLuminanceTransferRate_ ),
	};
		
	auto& Class = ReRegisterClass< ScnShaderToneMappingUniformBlockData >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "ToneMapping", BcFalse ) );
	Class.setFlags( bcRFF_POD );
}

//////////////////////////////////////////////////////////////////////////
// uncharted2Curve
BcF32 ScnShaderToneMappingUniformBlockData::uncharted2Curve( BcF32 X )
{
	const BcF32 A = ToneMappingA_;
	const BcF32 B = ToneMappingB_;
	const BcF32 C = ToneMappingC_;
	const BcF32 D = ToneMappingD_;
	const BcF32 E = ToneMappingE_;
	const BcF32 F = ToneMappingF_;
	return ( ( X * ( A * X + C * B ) + D * E ) / ( X * ( A * X + B ) + D * F ) ) - E / F;
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
