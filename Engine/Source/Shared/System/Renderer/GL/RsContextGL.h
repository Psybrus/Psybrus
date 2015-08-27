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
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsViewport.h"

#include "Base/BcMisc.h"

#if PLATFORM_LINUX || PLATFORM_OSX
#include <SDL2/SDL.h>
#endif

//////////////////////////////////////////////////////////////////////////
// RsBufferImplGL
struct RsBufferImplGL
{
	~RsBufferImplGL()
	{
		BcAssert( BufferData_ == nullptr );
	}

	GLuint Handle_ = 0;
	BcU8* BufferData_ = nullptr;
	BcU32 Version_ = 0;
};

//////////////////////////////////////////////////////////////////////////
// RsTextureImplGL
struct RsTextureImplGL
{
	~RsTextureImplGL()
	{
	}

	GLuint Handle_ = 0;
};

//////////////////////////////////////////////////////////////////////////
// RsProgramImplGL
struct RsProgramImplGL
{
	struct UniformEntry
	{
		enum class Type
		{
			UNKNOWN,
			UNIFORM_1IV,
			UNIFORM_1FV,
			UNIFORM_2FV,
			UNIFORM_3FV,
			UNIFORM_4FV,
			UNIFORM_MATRIX_4FV,
		};

		BcU32 BindingPoint_ = 0;
		Type Type_ = RsProgramImplGL::UniformEntry::Type::UNKNOWN;
		GLint Loc_ = 0;
		GLsizei Count_ = 0;
		size_t Offset_ = 0;
		size_t CachedOffset_ = 0;
		size_t Size_ = 0;

		// Used for redundancy checks.
		RsBuffer* Buffer_ = nullptr;
		BcU32 Version_ = 0;
	};

	GLuint Handle_ = 0;
	std::vector< UniformEntry > UniformEntries_;
	std::unique_ptr< BcU8[] > CachedUniforms_;
};

//////////////////////////////////////////////////////////////////////////
// RsContextGL
class RsContextGL:
	public RsContext
{
public:
	RsContextGL( OsClient* pClient, RsContextGL* pParent );
	virtual ~RsContextGL();
	
	virtual OsClient* getClient() const override;
	virtual const RsFeatures& getFeatures() const override;

	virtual BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const override;
	virtual RsShaderCodeType maxShaderCodeType( RsShaderCodeType CodeType ) const override;

	BcU32 getWidth() const override;
	BcU32 getHeight() const override;
	void beginFrame( BcU32 Width, BcU32 Height ) override;
	void endFrame() override;
	void takeScreenshot() override;

	bool createRenderState(
		RsRenderState* RenderState );
	bool destroyRenderState(
		RsRenderState* RenderState );

	bool createSamplerState(
		RsSamplerState* SamplerState );
	bool destroySamplerState(
		RsSamplerState* SamplerState );

	bool createFrameBuffer( 
		RsFrameBuffer* FrameBuffer );
	bool destroyFrameBuffer( 
		RsFrameBuffer* FrameBuffer );

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

	bool createVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration ) override;
	bool destroyVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration  ) override;

	void setDefaultState();
	void invalidateRenderState();
	void invalidateTextureState();
	void setRenderState( class RsRenderState* RenderState );
	void setSamplerState( BcU32 Slot, class RsSamplerState* SamplerState );
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
	void setFrameBuffer( class RsFrameBuffer* FrameBuffer );
	
	void flushState();

	void clear( 
		const RsColour& Colour,
		BcBool EnableClearColour,
		BcBool EnableClearDepth,
		BcBool EnableClearStencil ) override;
	void drawPrimitives( RsTopologyType TopologyType, BcU32 IndexOffset, BcU32 NoofIndices ) override;
	void drawIndexedPrimitives( RsTopologyType TopologyType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset ) override;
	void copyFrameBufferRenderTargetToTexture( RsFrameBuffer* FrameBuffer, BcU32 Idx, RsTexture* Texture ) override;
	void copyTextureToFrameBufferRenderTarget( RsTexture* Texture, RsFrameBuffer* FrameBuffer, BcU32 Idx ) override;

	void setViewport( const class RsViewport& Viewport ) override;
	void setScissorRect( BcS32 X, BcS32 Y, BcS32 Width, BcS32 Height ) override;

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

	void setRenderStateDesc( const RsRenderStateDesc& Desc, BcBool Force );

protected:
	virtual void create();
	virtual void update();
	virtual void destroy();	

#if PLATFORM_WINDOWS
	bool createProfile( RsOpenGLVersion Version, HGLRC ParentContext );
#endif

#if PLATFORM_LINUX || PLATFORM_OSX
	bool createProfile( RsOpenGLVersion Version, SDL_Window* Window );
#endif

private:
#if PLATFORM_WINDOWS
	HDC WindowDC_;
	HGLRC WindowRC_;
#endif

#if PLATFORM_LINUX || PLATFORM_OSX
	SDL_GLContext SDLGLContext_;
#endif

#if PLATFORM_ANDROID
	EGLDisplay EGLDisplay_;
	EGLConfig EGLConfig_; 
	EGLint EGLNumConfigs_;
	EGLint EGLFormat_;
	EGLSurface EGLSurface_;
	EGLContext EGLContext_;
	EGLint EGLWidth_;
	EGLint EGLHeight_;
	GLfloat EGLRatio_;
	struct ANativeWindow* EGLWindow_;
#endif

	RsContextGL* pParent_;
	OsClient* pClient_;

	BcU32 InsideBeginEnd_;
	BcU32 Width_;
	BcU32 Height_;

	RsOpenGLVersion Version_;

	BcBool ScreenshotRequested_;
	BcThreadId OwningThread_;
	BcU64 FrameCount_;

	struct TTextureStateValue
	{
		RsTexture* pTexture_;
		RsSamplerState* pSamplerState_;
		BcBool Dirty_;
	};

	enum
	{
		NOOF_RENDERSTATES = (BcU32)RsRenderStateType::MAX,
	};
	
	std::array< TTextureStateValue, MAX_TEXTURE_SLOTS > TextureStateValues_;

	// State setting.
	// TODO: Move into a seperate class.
	class RsRenderState* RenderState_;
	BcU64 LastRenderStateHandle_;
	RsRenderStateDesc BoundRenderStateDesc_;
	std::map< BcU64, RsRenderStateDesc > RenderStateMap_;

	// Frame buffer.
	BcBool DirtyFrameBuffer_;
	class RsFrameBuffer* FrameBuffer_;
	
	// Viewport.
	BcBool DirtyViewport_;
	RsViewport Viewport_;
	BcBool DirtyScissor_;
	BcS32 ScissorX_, ScissorY_, ScissorW_, ScissorH_;

	// Texture binding.
	std::array< BcU32, MAX_TEXTURE_SLOTS > TextureStateBinds_;
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

	BcU32 NoofDrawCalls_;
	BcU32 NoofRenderStateFlushes_;
	BcU32 NoofRenderStates_;
	BcU32 NoofSamplerStates_;
	BcU32 NoofBuffers_;
	BcU32 NoofTextures_;
	BcU32 NoofShaders_;
	BcU32 NoofPrograms_;

	GLuint TransferFBOs_[ 2 ];

};


#endif
