/**************************************************************************
*
* File:		RsContextGL.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCONTEXTGL_H__
#define __RSCONTEXTGL_H__

#include "System/Renderer/RsContext.h"
#include "System/Renderer/GL/RsGL.h"

#include "Base/BcMisc.h"

//////////////////////////////////////////////////////////////////////////
// RsContextGL
class RsContextGL:
	public RsContext
{
public:
	RsContextGL( OsClient* pClient, RsContextGL* pParent );
	virtual ~RsContextGL();
	
	virtual BcU32						getWidth() const;
	virtual BcU32						getHeight() const;

	void								swapBuffers();
	void								takeScreenshot();

	void								setDefaultState();
	void								invalidateRenderState();
	void								invalidateTextureState();
	void								setRenderState( eRsRenderState State, BcS32 Value, BcBool Force = BcFalse );
	BcS32								getRenderState( eRsRenderState State ) const;
	void								setTextureState( BcU32 Sampler, class RsTexture* pTexture, const RsTextureParams& Params, BcBool Force = BcFalse );

	void								flushState();

	void								clear( const RsColour& Colour );
	void								setProgram( class RsProgram* Program );
	void								setPrimitive( class RsPrimitive* Primitive );
	void								drawPrimitives( eRsPrimitiveType PrimitiveType, BcU32 Offset, BcU32 NoofIndices );
	void								drawIndexedPrimitives( eRsPrimitiveType PrimitiveType, BcU32 Offset, BcU32 NoofIndices );

private:
	protected:
	void								bindStencilFunc();
	void								bindStencilOp();
	void								bindBlendMode( eRsBlendingMode BlendMode );
	void								bindScissor();

protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	

#if PLATFORM_WINDOWS
	bool								createProfile( BcU32 Maj, BcU32 Min, BcBool IsCore, HGLRC ParentContext );
#endif

private:
#if PLATFORM_WINDOWS
	HDC WindowDC_;
	HGLRC WindowRC_;

	RsContextGL* pParent_;
#endif
	OsClient* pClient_;

	BcBool ScreenshotRequested_;
	BcThreadId OwningThread_;

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

	struct TVertexBufferSlot
	{
		class RsVertexBuffer*		VertexBuffer_;
	};

	enum
	{
		NOOF_RENDERSTATES = rsRS_MAX,
		NOOF_TEXTURESTATES = 8,
	};
		
	std::array< TRenderStateValue, NOOF_RENDERSTATES >		RenderStateValues_;
	std::array< TTextureStateValue, NOOF_TEXTURESTATES >	TextureStateValues_;

	// State setting.
	std::array< BcU32, NOOF_RENDERSTATES >					RenderStateBinds_;
	std::array< BcU32, NOOF_TEXTURESTATES >					TextureStateBinds_;
	BcU32							NoofRenderStateBinds_;
	BcU32							NoofTextureStateBinds_;	

	// VAO
	BcU32							GlobalVAO_;

	//
	BcBool							ProgramDirty_;
	BcBool							PrimitiveDirty_;
	RsProgram*						Program_;
	RsPrimitive*					Primitive_;
};


#endif
