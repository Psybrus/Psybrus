#pragma once

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
	void takeScreenshot( RsScreenshotFunc ScreenshotFunc ) override;

	bool createRenderState(
		RsRenderState* RenderState ) override;
	bool destroyRenderState(
		RsRenderState* RenderState ) override;

	bool createSamplerState(
		RsSamplerState* SamplerState ) override;
	bool destroySamplerState(
		RsSamplerState* SamplerState ) override;

	bool createFrameBuffer( 
		RsFrameBuffer* FrameBuffer ) override;
	bool destroyFrameBuffer( 
		RsFrameBuffer* FrameBuffer ) override;

	bool createBuffer( 
		RsBuffer* Buffer ) override;
	bool destroyBuffer( 
		RsBuffer* Buffer ) override;
	bool updateBuffer( 
		RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsResourceUpdateFlags Flags,
		RsBufferUpdateFunc UpdateFunc ) override;

	bool createTexture( 
		class RsTexture* Texture ) override;
	bool destroyTexture( 
		class RsTexture* Texture ) override;
	bool updateTexture( 
		class RsTexture* Texture,
		const struct RsTextureSlice& Slice,
		RsResourceUpdateFlags Flags,
		RsTextureUpdateFunc UpdateFunc ) override;

	bool createShader( class RsShader* Shader ) override;
	bool destroyShader( class RsShader* Shader ) override;
	bool createProgram( class RsProgram* Program ) override;
	bool destroyProgram( class RsProgram* Program ) override;
	bool createProgramBinding( class RsProgramBinding* ProgramBinding ) override;
	bool destroyProgramBinding( class RsProgramBinding* ProgramBinding ) override;
	bool createGeometryBinding( class RsGeometryBinding* GeometryBinding ) override;
	bool destroyGeometryBinding( class RsGeometryBinding* GeometryBinding ) override;
	bool createVertexDeclaration( class RsVertexDeclaration* VertexDeclaration ) override;
	bool destroyVertexDeclaration( class RsVertexDeclaration* VertexDeclaration ) override;
	
	void clear( 
		RsFrameBuffer* FrameBuffer,
		const RsColour& Colour,
		BcBool EnableClearColour,
		BcBool EnableClearDepth,
		BcBool EnableClearStencil ) override;
	void drawPrimitives( 
		RsGeometryBinding* GeometryBinding, 
		RsProgramBinding* ProgramBinding, 
		RsRenderState* RenderState,
		RsFrameBuffer* FrameBuffer, 
		const struct RsViewport* Viewport,
		const struct RsScissorRect* ScissorRect,
		RsTopologyType TopologyType, 
		BcU32 IndexOffset, BcU32 NoofIndices ) override;
	void drawIndexedPrimitives( 
		RsGeometryBinding* GeometryBinding, 
		RsProgramBinding* ProgramBinding, 
		RsRenderState* RenderState,
		RsFrameBuffer* FrameBuffer,
		const struct RsViewport* Viewport,
		const struct RsScissorRect* ScissorRect,
		RsTopologyType TopologyType, 
		BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset ) override;
	void copyFrameBufferRenderTargetToTexture( RsFrameBuffer* FrameBuffer, BcU32 Idx, RsTexture* Texture ) override;
	void copyTextureToFrameBufferRenderTarget( RsTexture* Texture, RsFrameBuffer* FrameBuffer, BcU32 Idx ) override;

	void dispatchCompute( class RsProgramBinding* ProgramBinding, BcU32 XGroups, BcU32 YGroups, BcU32 ZGroups ) override;

	void bindGeometry( const RsProgram* Program, const RsGeometryBinding* GeometryBinding );
	void bindFrameBuffer( const RsFrameBuffer* FrameBuffer, const RsViewport* Viewport, const RsScissorRect* ScissorRect );
	void bindRenderStateDesc( const RsRenderStateDesc& Desc, BcBool Force );
	void bindSRVs( const RsProgram* Program, const RsProgramBindingDesc& Bindings );
	void bindUAVs( const RsProgram* Program, const RsProgramBindingDesc& Bindings, GLbitfield& Barrier );
	void bindSamplerStates( const RsProgram* Program, const RsProgramBindingDesc& Bindings );
	void bindUniformBuffers( const RsProgram* Program, const RsProgramBindingDesc& Bindings );
	void unbindResource( const RsResource* Resource );

	const RsOpenGLVersion& getOpenGLVersion() const;

private:

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

	RsScreenshotFunc ScreenshotFunc_;
	BcThreadId OwningThread_;
	BcU64 FrameCount_;


	
	// State setting.
	// TODO: Move into a seperate class.
	class RsRenderState* RenderState_;
	RsRenderStateDesc BoundRenderStateDesc_;
	std::map< BcU64, RsRenderStateDesc > RenderStateMap_;

	// VAO
	BcU32 GlobalVAO_;

	std::array< bool, MAX_VERTEX_STREAMS > VertexBufferActiveState_;
	std::array< bool, MAX_VERTEX_STREAMS > VertexBufferActiveNextState_;

	GLbitfield MemoryBarrier_ = 0;

	struct TextureBindingInfo
	{
		const RsResource* Resource_ = nullptr;
		GLuint Texture_ = 0;
		GLenum Target_ = 0;
	};

	struct ImageBindingInfo
	{
		const RsResource* Resource_ = nullptr;
		GLuint Texture_ = 0;
		GLint Level_ = 0;
		GLboolean Layered_ = 0;
		GLint Layer_ = 0;
		GLenum Access_ = 0;
		GLenum Format_ = 0;
	};

	struct BufferBindingInfo
	{
		const RsResource* Resource_ = nullptr;
		GLuint Buffer_ = 0;
		GLintptr Offset_ = 0;
		GLsizei Size_ = 0;
	};

	struct SamplerBindingInfo
	{
		const RsResource* Resource_ = nullptr;
		GLuint Sampler_ = 0;
	};

	std::array< TextureBindingInfo, 32 > TextureBindingInfo_;
	std::array< ImageBindingInfo, 32 > ImageBindingInfo_;
	std::array< BufferBindingInfo, 32 > ShaderStorageBufferBindingInfo_;
	std::array< BufferBindingInfo, 32 > UniformBufferBindingInfo_;
	std::array< SamplerBindingInfo, 32 > SamplerBindingInfo_;

	BcBool UniformBuffersDirty_;

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
