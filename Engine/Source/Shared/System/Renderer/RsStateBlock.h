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

#include "RsTypes.h"
#include "RsTexture.h"

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
	void							invalidateRenderState( eRsRenderState State );
	
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
		
	TRenderStateValue				RenderStateValues_[ rsRS_MAX ];
	TTextureStateValue				TextureStateValues_[ rsTT_MAX ];

	// State setting.
	BcU32							RenderStateBinds_[ rsRS_MAX ];
	BcU32							TextureStateBinds_[ rsTT_MAX ];
	BcU32							NoofRenderStateBinds_;
	BcU32							NoofTextureStateBinds_;	
};

#endif
