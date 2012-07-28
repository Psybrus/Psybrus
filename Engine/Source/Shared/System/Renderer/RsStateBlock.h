/**************************************************************************
*
* File:		RsStateBlock.h
* Author:	Neil Richardson 
* Ver/Date:	16/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RsStateBlock_H__
#define __RsStateBlock_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsTexture.h"

//////////////////////////////////////////////////////////////////////////
// RsStateBlock
class RsStateBlock
{
public:
	RsStateBlock();
	virtual ~RsStateBlock();

	/**
	 * Bind state block.
	 */
	virtual void					bind() = 0;

	/**
	 * Set default state.
	 */
	void							setDefaultState();
	
	/**
	 * Invalidate render state.
	 */
	void							invalidateRenderState();
	
	/**
	 * Invalidate texture state.
	 */
	void							invalidateTextureState();
	
	/**
	 * Set render state.
	 */
	void							setRenderState( eRsRenderState State, BcS32 Value, BcBool Force = BcFalse );
	
	/**
	 * Set texture state.
	 */
	void							setTextureState( BcU32 Sampler, RsTexture* pTexture, const RsTextureParams& Params, BcBool Force = BcFalse );
	
protected:
	struct TRenderStateValue
	{
		BcS32						Value_;
		BcBool						Dirty_;
	};

	struct TTextureStateValue
	{
		RsTexture*					pTexture_;
		RsTextureParams				Params_;
		BcBool						Dirty_;
	};

	enum
	{
		NOOF_RENDERSTATES = rsRS_MAX,
		NOOF_TEXTURESTATES = 8
	};
		
	TRenderStateValue				RenderStateValues_[ NOOF_RENDERSTATES ];
	TTextureStateValue				TextureStateValues_[ NOOF_TEXTURESTATES ];

	// State setting.
	BcU32							RenderStateBinds_[ NOOF_RENDERSTATES ];
	BcU32							TextureStateBinds_[ NOOF_TEXTURESTATES ];
	BcU32							NoofRenderStateBinds_;
	BcU32							NoofTextureStateBinds_;	
};

#endif
