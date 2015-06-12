/**************************************************************************
*
* File:		RsRenderState.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Render state creation and management.
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsRenderState.h"

#include <type_traits>

//////////////////////////////////////////////////////////////////////////
// Assertions.
static_assert( std::is_trivially_copyable< RsRenderTargetBlendState >::value, "Unable to trivially copy RsRenderTargetBlendState" );
static_assert( std::is_trivially_copyable< RsBlendState >::value, "Unable to trivially copy RsBlendState" );
static_assert( std::is_trivially_copyable< RsStencilFaceState >::value, "Unable to trivially copy RsStencilFaceState" );
static_assert( std::is_trivially_copyable< RsStencilFaceState >::value, "Unable to trivially copy RsStencilFaceState" );
static_assert( std::is_trivially_copyable< RsDepthStencilState >::value, "Unable to trivially copy RsDepthStencilState" );
static_assert( std::is_trivially_copyable< RsRasteriserState >::value, "Unable to trivially copy RsRasteriserState" );
static_assert( std::is_trivially_copyable< RsRenderStateDesc >::value, "Unable to trivially copy RsRenderStateDesc" );

//////////////////////////////////////////////////////////////////////////
// RsRenderTargetBlendState
void RsRenderTargetBlendState::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Enable_", &RsRenderTargetBlendState::Enable_ ),
		new ReField( "SrcBlend_", &RsRenderTargetBlendState::SrcBlend_ ),
		new ReField( "DestBlend_", &RsRenderTargetBlendState::DestBlend_ ),
		new ReField( "BlendOp_", &RsRenderTargetBlendState::BlendOp_ ),
		new ReField( "SrcBlendAlpha_", &RsRenderTargetBlendState::SrcBlendAlpha_ ),
		new ReField( "DestBlendAlpha_", &RsRenderTargetBlendState::DestBlendAlpha_ ),
		new ReField( "BlendOpAlpha_", &RsRenderTargetBlendState::BlendOpAlpha_ ),
		new ReField( "WriteMask_", &RsRenderTargetBlendState::WriteMask_ ),
	};
		
	ReRegisterClass< RsRenderTargetBlendState >( Fields );
}

RsRenderTargetBlendState::RsRenderTargetBlendState( ReNoInit ):
	Enable_( BcFalse ),
	SrcBlend_( RsBlendType::ONE ),
	DestBlend_( RsBlendType::ONE ),
	BlendOp_( RsBlendOp::ADD ),
	SrcBlendAlpha_( RsBlendType::ONE ),
	DestBlendAlpha_( RsBlendType::ONE ),
	BlendOpAlpha_( RsBlendOp::ADD ),
	WriteMask_( 0xf )
{

}

RsRenderTargetBlendState::RsRenderTargetBlendState():
	Enable_( BcFalse ),
	SrcBlend_( RsBlendType::ONE ),
	DestBlend_( RsBlendType::ONE ),
	BlendOp_( RsBlendOp::ADD ),
	SrcBlendAlpha_( RsBlendType::ONE ),
	DestBlendAlpha_( RsBlendType::ONE ),
	BlendOpAlpha_( RsBlendOp::ADD ),
	WriteMask_( 0xf )
{

}

//////////////////////////////////////////////////////////////////////////
// RsBlendState
void RsBlendState::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "RenderTarget_", &RsBlendState::RenderTarget_ ),
	};
		
	ReRegisterClass< RsBlendState >( Fields );
}

RsBlendState::RsBlendState( ReNoInit ):
	RenderTarget_()
{

}

RsBlendState::RsBlendState():
	RenderTarget_()
{

}

//////////////////////////////////////////////////////////////////////////
// RsStencilFaceState
void RsStencilFaceState::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Fail_", &RsStencilFaceState::Fail_ ),
		new ReField( "DepthFail_", &RsStencilFaceState::DepthFail_ ),
		new ReField( "Pass_", &RsStencilFaceState::Pass_ ),
		new ReField( "Func_", &RsStencilFaceState::Func_ ),
		new ReField( "Mask_", &RsStencilFaceState::Mask_ ),
	};
		
	ReRegisterClass< RsStencilFaceState >( Fields );
}

RsStencilFaceState::RsStencilFaceState( ReNoInit ):
	Fail_( RsStencilOp::KEEP ),
	DepthFail_( RsStencilOp::KEEP ),
	Pass_( RsStencilOp::KEEP ),
	Func_( RsCompareMode::ALWAYS ),
	Mask_( 0 )
{

}

RsStencilFaceState::RsStencilFaceState():
	Fail_( RsStencilOp::KEEP ),
	DepthFail_( RsStencilOp::KEEP ),
	Pass_( RsStencilOp::KEEP ),
	Func_( RsCompareMode::ALWAYS ),
	Mask_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// RsDepthStencilState
void RsDepthStencilState::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "DepthTestEnable_", &RsDepthStencilState::DepthTestEnable_ ),
		new ReField( "DepthWriteEnable_", &RsDepthStencilState::DepthWriteEnable_ ),
		new ReField( "DepthFunc_", &RsDepthStencilState::DepthFunc_ ),
		new ReField( "StencilEnable_", &RsDepthStencilState::StencilEnable_ ),
		new ReField( "StencilRead_", &RsDepthStencilState::StencilRead_ ),
		new ReField( "StencilWrite_", &RsDepthStencilState::StencilWrite_ ),
		new ReField( "StencilFront_", &RsDepthStencilState::StencilFront_ ),
		new ReField( "StencilBack_", &RsDepthStencilState::StencilBack_ ),
		new ReField( "StencilRef_", &RsDepthStencilState::StencilRef_ ),
	};
	
	ReRegisterClass< RsDepthStencilState >( Fields );
}

RsDepthStencilState::RsDepthStencilState( ReNoInit ):
	DepthTestEnable_( BcFalse ),
	DepthWriteEnable_( BcFalse ),
	DepthFunc_( RsCompareMode::LESS ),
	StencilEnable_( BcFalse ),
	StencilFront_(),
	StencilBack_(),
	StencilRef_( 0x0 ),
	StencilRead_( 0x0 ),
	StencilWrite_( 0x0 ),
	Padding_( 0 )
{

}

RsDepthStencilState::RsDepthStencilState():
	DepthTestEnable_( BcFalse ),
	DepthWriteEnable_( BcFalse ),
	DepthFunc_( RsCompareMode::LESS ),
	StencilEnable_( BcFalse ),
	StencilFront_(),
	StencilBack_(),
	StencilRef_( 0 ),
	StencilRead_( 0x0 ),
	StencilWrite_( 0x0 ),
	Padding_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// RsRasteriserState
void RsRasteriserState::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "FillMode_", &RsRasteriserState::FillMode_ ),
		new ReField( "CullMode_", &RsRasteriserState::CullMode_ ),
		new ReField( "DepthBias_", &RsRasteriserState::DepthBias_ ),
		new ReField( "SlopeScaledDepthBias_", &RsRasteriserState::SlopeScaledDepthBias_ ),
		new ReField( "DepthClipEnable_", &RsRasteriserState::DepthClipEnable_ ),
		new ReField( "ScissorEnable_", &RsRasteriserState::ScissorEnable_ ),
		new ReField( "AntialiasedLineEnable_", &RsRasteriserState::AntialiasedLineEnable_ ),
	};
		
	ReRegisterClass< RsRasteriserState >( Fields );
}

RsRasteriserState::RsRasteriserState( ReNoInit ):
	FillMode_( RsFillMode::SOLID ),
	CullMode_( RsCullMode::NONE ),
	DepthBias_( 0.0f ),
	SlopeScaledDepthBias_( 0.0f ),
	DepthClipEnable_( BcTrue ),
	ScissorEnable_( BcFalse ),
	AntialiasedLineEnable_( BcTrue )
{

}

RsRasteriserState::RsRasteriserState():
	FillMode_( RsFillMode::SOLID ),
	CullMode_( RsCullMode::NONE ),
	DepthBias_( 0.0f ),
	SlopeScaledDepthBias_( 0.0f ),
	DepthClipEnable_( BcTrue ),
	ScissorEnable_( BcFalse ),
	AntialiasedLineEnable_( BcTrue )
{

}

//////////////////////////////////////////////////////////////////////////
// RsRenderStateDesc
void RsRenderStateDesc::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "BlendState_", &RsRenderStateDesc::BlendState_ ),
		new ReField( "DepthStencilState_", &RsRenderStateDesc::DepthStencilState_ ),
		new ReField( "RasteriserState_", &RsRenderStateDesc::RasteriserState_ ),
	};
		
	ReRegisterClass< RsRenderStateDesc >( Fields );
}

RsRenderStateDesc::RsRenderStateDesc( ReNoInit ):
	BlendState_( NOINIT ),
	DepthStencilState_( NOINIT ),
	RasteriserState_( NOINIT )
{

}

RsRenderStateDesc::RsRenderStateDesc():
	BlendState_(),
	DepthStencilState_(),
	RasteriserState_()
{

}

RsRenderStateDesc::RsRenderStateDesc( 
		const RsBlendState& BlendState, 
		const RsDepthStencilState& DepthStencilState, 
		const RsRasteriserState& RasteriserState ):
	BlendState_( BlendState ),
	DepthStencilState_( DepthStencilState ),
	RasteriserState_( RasteriserState )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsRenderState::RsRenderState( class RsContext* pContext, const RsRenderStateDesc& Desc ):
	RsResource( pContext ),
	Desc_( Desc )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsRenderState::~RsRenderState()
{

}

//////////////////////////////////////////////////////////////////////////
// getDesc
const RsRenderStateDesc& RsRenderState::getDesc() const
{
	return Desc_;
}
