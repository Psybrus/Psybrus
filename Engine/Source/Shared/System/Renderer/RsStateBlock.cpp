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

#include "RsStateBlock.h"
#include "BcMemory.h"

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
	setRenderState( rsRS_DEPTH_WRITE_ENABLE,		BcFalse,		BcTrue );
	setRenderState( rsRS_DEPTH_TEST_ENABLE,			BcFalse,		BcTrue );
	setRenderState( rsRS_DEPTH_TEST_COMPARE,		rsCM_NEVER,		BcTrue );
	setRenderState( rsRS_DEPTH_BIAS,				0,				BcTrue );
	setRenderState( rsRS_ALPHA_TEST_ENABLE,			BcFalse,		BcTrue );
	setRenderState( rsRS_ALPHA_TEST_COMPARE,		rsCM_NEVER,		BcTrue );
	setRenderState( rsRS_ALPHA_TEST_THRESHOLD,		0,				BcTrue );
	setRenderState( rsRS_BLEND_MODE,				rsBM_NONE,		BcTrue );
	
	// Setup default texture states.
	RsTextureParams TextureParams = 
	{
		rsTFM_NEAREST, rsTFM_NEAREST, rsTSM_WRAP, rsTSM_WRAP
	};

	for( BcU32 Sampler = 0; Sampler < rsTT_MAX; ++Sampler )
	{
		setTextureState( Sampler, NULL, TextureParams, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsStateBlock::invalidateRenderState( eRsRenderState State )
{
	TRenderStateValue& RenderStateValue = RenderStateValues_[ State ];

	RenderStateValue.Dirty_ = BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsStateBlock::invalidateTextureState()
{
	for( BcU32 Idx = 0; Idx < rsTT_MAX; ++Idx )
	{
		TTextureStateValue& TextureStateValue = TextureStateValues_[ Idx ];
		
		TextureStateValue.Dirty_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsStateBlock::setRenderState( eRsRenderState State, BcS32 Value, BcBool Force )
{
	if( State < rsRS_MAX )
	{
		TRenderStateValue& RenderStateValue = RenderStateValues_[ State ];
		
		const BcBool WasDirty = RenderStateValue.Dirty_;
		
		RenderStateValue.Dirty_ |= ( RenderStateValue.Value_ != Value ) || Force;
		RenderStateValue.Value_ = Value;
		
		// If it wasn't dirty, we need to set it.
		if( WasDirty == BcFalse && RenderStateValue.Dirty_ == BcTrue )
		{
			BcAssert( NoofRenderStateBinds_ < rsRS_MAX );
			RenderStateBinds_[ NoofRenderStateBinds_++ ] = State;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTextureState
void RsStateBlock::setTextureState( BcU32 Sampler, RsTexture* pTexture, const RsTextureParams& Params, BcBool Force )
{
	if( Sampler < rsTT_MAX )
	{
		TTextureStateValue& TextureStateValue = TextureStateValues_[ Sampler ];
		
		const BcBool WasDirty = TextureStateValue.Dirty_;
		
		TextureStateValue.Dirty_ |= ( TextureStateValue.pTexture_ != pTexture || TextureStateValue.Params_ != Params ) || Force;
		TextureStateValue.pTexture_ = pTexture;
		TextureStateValue.Params_ = Params;
	
		// If it wasn't dirty, we need to set it.
		if( WasDirty == BcFalse && TextureStateValue.Dirty_ == BcTrue )
		{
			BcAssert( NoofTextureStateBinds_ < rsTT_MAX );
			TextureStateBinds_[ NoofTextureStateBinds_++ ] = Sampler;
		}
	}
}
