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

#include "System/Renderer/RsBuffer.h"

#include "Base/BcMisc.h"

//////////////////////////////////////////////////////////////////////////
// RsContextGL
class RsContextGL:
	public RsContext
{
public:
	RsContextGL( OsClient* pClient, RsContextGL* pParent );
	virtual ~RsContextGL();
	
	virtual BcU32 getWidth() const;
	virtual BcU32 getHeight() const;
	virtual BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const;
	virtual RsShaderCodeType maxShaderCodeType( RsShaderCodeType CodeType ) const;

	void swapBuffers();
	void takeScreenshot();

	bool createBuffer( 
		RsBuffer* Buffer );
	bool destroyBuffer( 
		RsBuffer* Buffer );
	bool updateBuffer( 
		RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsBufferUpdateFlags Flags,
		RsUpdateBufferFunc UpdateFunc );

	void setDefaultState();
	void invalidateRenderState();
	void invalidateTextureState();
	void setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force = BcFalse );
	BcS32 getRenderState( RsRenderStateType State ) const;
	void setTextureState( BcU32 Sampler, class RsTexture* pTexture, const RsTextureParams& Params, BcBool Force = BcFalse );
	void setProgram( class RsProgram* Program );
	void setIndexBuffer( class RsBuffer* IndexBuffer );
	void setVertexBuffer( BcU32 StreamIdx, class RsVertexBuffer* VertexBuffer );
	void setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration );
	
	void flushState();

	void clear( const RsColour& Colour );
	void drawPrimitives( RsTopologyType TopologyType, BcU32 IndexOffset, BcU32 NoofIndices );
	void drawIndexedPrimitives( RsTopologyType TopologyType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset );

	void setRenderTarget( class RsRenderTarget* RenderTarget );
	void setViewport( class RsViewport& Viewport );

	const RsOpenGLVersion& getOpenGLVersion() const;

private:
	void bindStencilFunc();
	void bindStencilOp();
	void bindBlendMode( RsBlendingMode BlendMode );
	void bindScissor();

protected:
	virtual void create();
	virtual void update();
	virtual void destroy();	

#if PLATFORM_WINDOWS
	bool createProfile( RsOpenGLVersion Version, HGLRC ParentContext );
#endif

private:
#if PLATFORM_WINDOWS
	HDC WindowDC_;
	HGLRC WindowRC_;

	RsContextGL* pParent_;
#endif
	OsClient* pClient_;

	RsOpenGLVersion Version_;

	BcBool ScreenshotRequested_;
	BcThreadId OwningThread_;

	struct TRenderStateValue
	{
		BcS32 Value_;
		BcBool Dirty_;
	};

	struct TTextureStateValue
	{
		RsTexture* pTexture_;
		RsTextureParams Params_;
		BcBool Dirty_;
	};

	struct TVertexBufferSlot
	{
		class RsVertexBuffer* VertexBuffer_;
	};

	enum
	{
		NOOF_RENDERSTATES = RsRenderStateType::MAX,
		NOOF_TEXTURESTATES = 8,
	};
		
	std::array< TRenderStateValue, NOOF_RENDERSTATES > RenderStateValues_;
	std::array< TTextureStateValue, NOOF_TEXTURESTATES > TextureStateValues_;

	// State setting.
	std::array< BcU32, NOOF_RENDERSTATES > RenderStateBinds_;
	std::array< BcU32, NOOF_TEXTURESTATES > TextureStateBinds_;
	BcU32 NoofRenderStateBinds_;
	BcU32 NoofTextureStateBinds_;	

	// VAO
	BcU32 GlobalVAO_;

	//
	BcBool ProgramDirty_;
	BcBool BindingsDirty_; // TODO: Break down to be more fine grained.
	RsProgram* Program_;

	RsBuffer* IndexBuffer_;
	std::array< RsVertexBuffer*, MAX_VERTEX_STREAMS > VertexBuffers_;
	RsVertexDeclaration* VertexDeclaration_;
};


#endif
