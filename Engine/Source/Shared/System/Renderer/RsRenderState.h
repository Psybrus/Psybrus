/**************************************************************************
*
* File:		RsRenderStateType.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Render state creation and management.
*		
*
*
* 
**************************************************************************/

#ifndef __RENDERSTATE_H__
#define __RENDERSTATE_H__

#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsRenderTargetBlendState
struct RsRenderTargetBlendState
{
	BcBool			Enable_;
	RsBlendType	SrcBlend_;
	RsBlendType	DestBlend_;
	RsBlendOp		BlendOp_;
	RsBlendType	SrcBlendAlpha_;
	RsBlendType	DestBlendAlpha_;
	RsBlendOp		BlendOpAlpha_;
	BcU8			WriteMask_;
};

//////////////////////////////////////////////////////////////////////////
// RsBlendState
struct RsBlendState
{
	RsRenderTargetBlendState	RenderTarget_[ 8 ];
};

//////////////////////////////////////////////////////////////////////////
// RsStencilFaceState
struct RsStencilFaceState
{
	RsStencilOp	Fail_;
	RsStencilOp	DepthFail_;
	RsStencilOp	PassFail_;
	RsCompareMode	Func_;
};

//////////////////////////////////////////////////////////////////////////
// RsDepthStencilState
struct RsDepthStencilState
{
	BcBool			DepthTestEnable_;
	BcBool			DepthWriteEnable_;
	RsCompareMode	DepthFunc_;
	BcBool			StencilEnable_;
	BcU8			StencilRead_;
	BcU8			StencilWrite_;
	RsStencilFaceState StencilFront_;
	RsStencilFaceState StencilBack_;
};

//////////////////////////////////////////////////////////////////////////
// RsRasteriserState
struct RsRasteriserState
{
	BcU32			FillMode_;
	BcU32			CullMode_;
	BcF32			DepthBias_;
	BcF32			SlopeScaledDepthBias_;
	BcBool			DepthClipEnable_;
	BcBool			ScissorEnable_;
	BcBool			AntialiasedLineEnable_;
};

//////////////////////////////////////////////////////////////////////////
// RsRenderState
union RsRenderState
{
	RsRenderState()
	{
	
	}
	
	RsRenderState( BcU16 Value ):
		Value_( Value )
	{
	
	}
	
	RsRenderState( const RsRenderState& State ):
		Value_( State.Value_ )
	{
	
	}

	struct
	{
		BcU16		BlendStateIdx_			: 5;		// Max of 32 blend states.
		BcU16		DepthStencilStateIdx_	: 5;		// Max of 32 depth stencil states.
		BcU16		RasteriserStateIdx_		: 5;		// Max of 32 rasteriser states.
		BcU16		Spare_					: 1;		// Spare bit for something?
	};

	BcU16			Value_;
};


#endif

