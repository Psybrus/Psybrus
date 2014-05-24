/**************************************************************************
*
* File:		RsRenderTarget.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Render target.
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsRenderTarget.h"

#include "Base/BcMemory.h"

RsRenderTargetDesc::RsRenderTargetDesc(
						BcU32 Width,
						BcU32 Height )
{
	// zero off all colour formats.
	BcMemZero( &ColourFormats_[ 0 ], sizeof( ColourFormats_ ) );
	Width_ = Width;
	Height_ = Height;
}

RsRenderTargetDesc& RsRenderTargetDesc::renderSurface( BcU32 Idx, RsColourFormat Format )
{
	ColourFormats_[ Idx ] = Format;
	return *this;
}

RsRenderTargetDesc& RsRenderTargetDesc::depthStencilSurface( RsDepthStencilFormat Format )
{
	DepthStencilFormat_ = Format;
	return *this;
}