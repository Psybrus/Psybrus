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

RsRenderTargetDesc::RsRenderTargetDesc( eRsColourFormat ColourFormat, 
						eRsDepthStencilFormat DepthStencilFormat,
						BcU32 Width,
						BcU32 Height )
{
	// zero off all colour formats.
	BcMemZero( &ColourFormats_[ 0 ], sizeof( ColourFormats_ ) );

	// setup everything.
	ColourFormats_[ 0 ] = ColourFormat;
	DepthStencilFormat_ = DepthStencilFormat;
	Width_ = Width;
	Height_ = Height;
}

RsRenderTargetDesc::RsRenderTargetDesc( BcU32 NoofColourFormats,
						eRsColourFormat* ColourFormats, 
						eRsDepthStencilFormat DepthStencilFormat,
						BcU32 Width,
						BcU32 Height )
{
	BcAssert( NoofColourFormats < (BcU32)ColourFormats_.size() );
	// zero off all colour formats.
	BcMemZero( &ColourFormats_[ 0 ], sizeof( ColourFormats_ ) );

	// setup everything.
	for( BcU32 Idx = 0; Idx < NoofColourFormats; ++Idx )
	{
		ColourFormats_[ Idx ] = ColourFormats[ Idx ];
	}
	
	DepthStencilFormat_ = DepthStencilFormat;
	Width_ = Width;
	Height_ = Height;
}
