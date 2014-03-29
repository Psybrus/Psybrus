/**************************************************************************
*
* File:		RsStateBlock.cpp
* Author:	Neil Richardson 
* Ver/Date:	16/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsStateBlock.h"
#include "Base/BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsStateBlock::RsStateBlock()
{
	BcMemZero( &RenderStateValues_[ 0 ], sizeof( RenderStateValues_ ) );
	BcMemZero( &TextureStateValues_[ 0 ], sizeof( TextureStateValues_ ) );
	BcMemZero( &RenderStateBinds_[ 0 ], sizeof( RenderStateBinds_ ) );
	BcMemZero( &TextureStateBinds_[ 0 ], sizeof( TextureStateBinds_ ) );

	NoofRenderStateBinds_ = 0;
	NoofTextureStateBinds_ = 0;
	
	setDefaultState();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsStateBlock::~RsStateBlock()
{
	
}

//////////////////////////////////////////////////////////////////////////
// setDefaultState
void RsStateBlock::setDefaultState()
{
	// Setup default render states.
	setRenderState( rsRS_DEPTH_WRITE_ENABLE,		1,					BcTrue );
	setRenderState( rsRS_DEPTH_TEST_ENABLE,			0,					BcTrue );
	setRenderState( rsRS_DEPTH_TEST_COMPARE,		0,					BcTrue );
	setRenderState( rsRS_DEPTH_BIAS,				0,					BcTrue );
	setRenderState( rsRS_STENCIL_WRITE_MASK,		0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_ENABLE,		0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_FUNC_COMPARE,	0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_FUNC_REF,		0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_FUNC_MASK,	0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_OP_SFAIL,		0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_OP_DPFAIL,	0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_OP_DPPASS,	0,					BcTrue );
	setRenderState( rsRS_COLOR_WRITE_RED_ENABLE,	1,					BcTrue );
	setRenderState( rsRS_COLOR_WRITE_GREEN_ENABLE,	1,					BcTrue );
	setRenderState( rsRS_COLOR_WRITE_BLUE_ENABLE,	1,					BcTrue );
	setRenderState( rsRS_COLOR_WRITE_ALPHA_ENABLE,	1,					BcTrue );
	setRenderState( rsRS_BLEND_MODE,				0,					BcTrue );
	
	// Setup default texture states.
	RsTextureParams TextureParams = 
	{
		rsTFM_LINEAR, rsTFM_LINEAR, rsTSM_WRAP, rsTSM_WRAP
	};

	for( BcU32 Sampler = 0; Sampler < NOOF_TEXTURESTATES; ++Sampler )
	{
		setTextureState( Sampler, NULL, TextureParams, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsStateBlock::invalidateRenderState()
{
	NoofRenderStateBinds_ = 0;
	for( BcU32 Idx = 0; Idx < NOOF_RENDERSTATES; ++Idx )
	{
		TRenderStateValue& RenderStateValue = RenderStateValues_[ Idx ];

		RenderStateValue.Dirty_ = BcTrue;
		
		BcAssert( NoofRenderStateBinds_ < NOOF_RENDERSTATES );
		RenderStateBinds_[ NoofRenderStateBinds_++ ] = Idx;
	}
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsStateBlock::invalidateTextureState()
{
	NoofTextureStateBinds_ = 0;
	for( BcU32 Idx = 0; Idx < NOOF_TEXTURESTATES; ++Idx )
	{
		TTextureStateValue& TextureStateValue = TextureStateValues_[ Idx ];
		
		TextureStateValue.Dirty_ = BcTrue;
		
		BcAssert( NoofTextureStateBinds_ < NOOF_TEXTURESTATES );
		TextureStateBinds_[ NoofTextureStateBinds_++ ] = Idx;
	}
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsStateBlock::setRenderState( eRsRenderState State, BcS32 Value, BcBool Force )
{
	if( State < NOOF_RENDERSTATES )
	{
		TRenderStateValue& RenderStateValue = RenderStateValues_[ State ];
		
		const BcBool WasDirty = RenderStateValue.Dirty_;
		
		RenderStateValue.Dirty_ |= ( RenderStateValue.Value_ != Value ) || Force;
		RenderStateValue.Value_ = Value;
		
		// If it wasn't dirty, we need to set it.
		if( WasDirty == BcFalse && RenderStateValue.Dirty_ == BcTrue )
		{
			BcAssert( NoofRenderStateBinds_ < NOOF_RENDERSTATES );
			RenderStateBinds_[ NoofRenderStateBinds_++ ] = State;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getRenderState
BcS32 RsStateBlock::getRenderState( eRsRenderState State ) const
{
	if( State < NOOF_RENDERSTATES )
	{
		const TRenderStateValue& RenderStateValue = RenderStateValues_[ State ];

		return RenderStateValue.Value_;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// setTextureState
void RsStateBlock::setTextureState( BcU32 Sampler, RsTexture* pTexture, const RsTextureParams& Params, BcBool Force )
{
	if( Sampler < NOOF_TEXTURESTATES )
	{
		TTextureStateValue& TextureStateValue = TextureStateValues_[ Sampler ];
		
		const BcBool WasDirty = TextureStateValue.Dirty_;
		
		TextureStateValue.Dirty_ |= ( TextureStateValue.pTexture_ != pTexture || TextureStateValue.Params_ != Params ) || Force;
		TextureStateValue.pTexture_ = pTexture;
		TextureStateValue.Params_ = Params;
	
		// If it wasn't dirty, we need to set it.
		if( WasDirty == BcFalse && TextureStateValue.Dirty_ == BcTrue )
		{
			BcAssert( NoofTextureStateBinds_ < NOOF_TEXTURESTATES );
			TextureStateBinds_[ NoofTextureStateBinds_++ ] = Sampler;
		}
	}
}
