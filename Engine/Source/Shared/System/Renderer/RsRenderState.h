/**************************************************************************
*
* File:		RsRenderState.h
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
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsRenderTargetBlendState
struct RsRenderTargetBlendState
{
	REFLECTION_DECLARE_BASIC( RsRenderTargetBlendState );
	RsRenderTargetBlendState();

	BcBool Enable_;
	RsBlendType SrcBlend_;
	RsBlendType DestBlend_;
	RsBlendOp BlendOp_;
	RsBlendType SrcBlendAlpha_;
	RsBlendType DestBlendAlpha_;
	RsBlendOp BlendOpAlpha_;
	BcU8 WriteMask_;
};

//////////////////////////////////////////////////////////////////////////
// RsBlendState
struct RsBlendState
{
	REFLECTION_DECLARE_BASIC( RsBlendState );
	RsBlendState();

	RsRenderTargetBlendState RenderTarget_[ 8 ];
};

//////////////////////////////////////////////////////////////////////////
// RsStencilFaceState
struct RsStencilFaceState
{
	REFLECTION_DECLARE_BASIC( RsStencilFaceState );
	RsStencilFaceState();

	RsStencilOp Fail_;
	RsStencilOp DepthFail_;
	RsStencilOp PassFail_;
	RsCompareMode Func_;
};

//////////////////////////////////////////////////////////////////////////
// RsDepthStencilState
struct RsDepthStencilState
{
	REFLECTION_DECLARE_BASIC( RsDepthStencilState );
	RsDepthStencilState();

	BcBool DepthTestEnable_;
	BcBool DepthWriteEnable_;
	RsCompareMode DepthFunc_;
	BcBool StencilEnable_;
	BcU8 StencilRead_;
	BcU8 StencilWrite_;
	RsStencilFaceState StencilFront_;
	RsStencilFaceState StencilBack_;
};

//////////////////////////////////////////////////////////////////////////
// RsRasteriserState
struct RsRasteriserState
{
	REFLECTION_DECLARE_BASIC( RsRasteriserState );
	RsRasteriserState();

	RsFillMode FillMode_;
	RsCullMode CullMode_;
	BcF32 DepthBias_;
	BcF32 SlopeScaledDepthBias_;
	BcBool DepthClipEnable_;
	BcBool ScissorEnable_;
	BcBool AntialiasedLineEnable_;
};

//////////////////////////////////////////////////////////////////////////
// RsRenderStateDesc
struct RsRenderStateDesc
{
	REFLECTION_DECLARE_BASIC( RsRenderStateDesc );
	
	RsRenderStateDesc();
	RsRenderStateDesc( 
		const RsBlendState& BlendState, 
		const RsDepthStencilState& DepthStencilState, 
		const RsRasteriserState& RasteriserState );

	RsBlendState BlendState_;
	RsDepthStencilState DepthStencilState_;
	RsRasteriserState RasteriserState_;
};

//////////////////////////////////////////////////////////////////////////
// RsRenderState
class RsRenderState:
	public RsResource
{
	RsRenderState( class RsContext* pContext, const RsRenderStateDesc& Desc );
	virtual ~RsRenderState();

	/**
	 * Get desc.
	 */
	 const RsRenderStateDesc& getDesc() const;

private:
	RsRenderStateDesc Desc_;
};


#endif

