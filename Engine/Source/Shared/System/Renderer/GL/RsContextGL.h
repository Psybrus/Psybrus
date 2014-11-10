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

#if PLATFORM_LINUX
#include <SDL2/SDL.h>
#endif

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

	void presentBackBuffer();
	void takeScreenshot();

	bool createRenderState(
		RsRenderState* RenderState );
	bool destroyRenderState(
		RsRenderState* RenderState );

	bool createBuffer( 
		RsBuffer* Buffer );
	bool destroyBuffer( 
		RsBuffer* Buffer );
	bool updateBuffer( 
		RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsResourceUpdateFlags Flags,
		RsBufferUpdateFunc UpdateFunc );

	bool createTexture( 
		class RsTexture* Texture );
	bool destroyTexture( 
		class RsTexture* Texture );
	bool updateTexture( 
		class RsTexture* Texture,
		const struct RsTextureSlice& Slice,
		RsResourceUpdateFlags Flags,
		RsTextureUpdateFunc UpdateFunc );

	bool createShader(
		class RsShader* Shader );
	bool destroyShader(
		class RsShader* Shader );

	bool createProgram(
		class RsProgram* Program );
	bool destroyProgram(
		class RsProgram* Program );

	void setDefaultState();
	void invalidateRenderState();
	void invalidateTextureState();
	void setRenderState( class RsRenderState* RenderState );
	void setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force = BcFalse );
	BcS32 getRenderState( RsRenderStateType State ) const;
	void setSamplerState( BcU32 Slot, const RsTextureParams& Params, BcBool Force = BcFalse );
	void setTexture( BcU32 Slot, class RsTexture* pTexture, BcBool Force = BcFalse );
	void setProgram( class RsProgram* Program );
	void setIndexBuffer( class RsBuffer* IndexBuffer );
	void setVertexBuffer( 
		BcU32 StreamIdx, 
		class RsBuffer* VertexBuffer,
		BcU32 Stride );
	void setUniformBuffer( 
		BcU32 SlotIdx, 
		class RsBuffer* UniformBuffer );
	void setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration );
	
	void flushState();

	void clear( const RsColour& Colour );
	void drawPrimitives( RsTopologyType TopologyType, BcU32 IndexOffset, BcU32 NoofIndices );
	void drawIndexedPrimitives( RsTopologyType TopologyType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset );

	void setViewport( class RsViewport& Viewport );

	const RsOpenGLVersion& getOpenGLVersion() const;

private:
	void bindStencilFunc();
	void bindStencilOp();
	void bindBlendMode( RsBlendingMode BlendMode );
	void bindScissor();

	void loadTexture( 
		RsTexture* Texture, 
		const RsTextureSlice& Slice,
		BcBool Bind, 
		BcU32 DataSize,
		void* Data );

protected:
	virtual void create();
	virtual void update();
	virtual void destroy();	

#if PLATFORM_WINDOWS
	bool createProfile( RsOpenGLVersion Version, HGLRC ParentContext );
#endif

#if PLATFORM_LINUX
	bool createProfile( RsOpenGLVersion Version, SDL_Window* Window );
#endif

private:
#if PLATFORM_WINDOWS
	HDC WindowDC_;
	HGLRC WindowRC_;
#endif

#if PLATFORM_LINUX
	SDL_GLContext SDLGLContext_;
#endif

	RsContextGL* pParent_;
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

	enum
	{
		NOOF_RENDERSTATES = (BcU32)RsRenderStateType::MAX,
	};
		
	std::array< TRenderStateValue, NOOF_RENDERSTATES > RenderStateValues_;
	std::array< TTextureStateValue, MAX_TEXTURE_SLOTS > TextureStateValues_;

	// State setting.
	std::array< BcU32, NOOF_RENDERSTATES > RenderStateBinds_;
	std::array< BcU32, MAX_TEXTURE_SLOTS > TextureStateBinds_;
	BcU32 NoofRenderStateBinds_;
	BcU32 NoofTextureStateBinds_;	

	// VAO
	BcU32 GlobalVAO_;

	//
	BcBool ProgramDirty_;
	BcBool BindingsDirty_; // TODO: Break down to be more fine grained.
	RsProgram* Program_;

	RsBuffer* IndexBuffer_;


	struct VertexBufferBinding
	{
		RsBuffer* Buffer_;
		BcU32 Stride_;
	};

	struct UniformBufferBinding
	{
		RsBuffer* Buffer_;
	};

	std::array< VertexBufferBinding, MAX_VERTEX_STREAMS > VertexBuffers_;
	std::array< UniformBufferBinding, MAX_UNIFORM_SLOTS > UniformBuffers_;
	RsVertexDeclaration* VertexDeclaration_;
};


#endif
