/**************************************************************************
*
* File:		RsContext.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCONTEXT_H__
#define __RSCONTEXT_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsFeatures.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class OsClient;

//////////////////////////////////////////////////////////////////////////
// RsContext
class RsContext:
	public RsResource
{
public:
	enum
	{
		MAX_RENDER_TARGETS = 8,
		MAX_VERTEX_STREAMS = 16,
		MAX_UNIFORM_SLOTS = 16,
		MAX_TEXTURE_SLOTS = 16,
		MAX_SAMPLER_SLOTS = 16,
	};

public:
	RsContext( RsContext* pParentContext ):
		RsResource( RsResourceType::CONTEXT, pParentContext )
	{}
	virtual ~RsContext();

	/**
	 * Get client.
	 * Thread safe.
	 */
	virtual OsClient* getClient() const = 0;

	/**
	 * Get features.
	 * Thread safe.
	 */
	virtual const RsFeatures& getFeatures() const = 0;

	/** 
	 * Is shader code type supported?
	 * Thread safe.
	 * TODO: Deprecate, use getFeatures.
	 */
	virtual BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const = 0;

	/** 
	 * Get max shader type supported.
	 * Thread safe.
	 * TODO: Deprecate, use getFeatures.
	 */
	virtual RsShaderCodeType maxShaderCodeType( RsShaderCodeType CodeType ) const = 0;

	/**
 	 * Get width.
	 * @pre Between beginFrame/endFrame calls. 
	 */
	virtual BcU32 getWidth() const = 0;

	/**
 	 * Get height.
	 * @pre Between beginFrame/endFrame calls. 
	 */
	virtual BcU32 getHeight() const = 0;

	/**
	 * Get back buffer frame buffer.
	 */
	virtual class RsFrameBuffer* getBackBuffer() const = 0;

	/**
	 *	Resize backbuffer.
	 *	@pre Must be outside of beginFrame/endFrame block.
	 */
	virtual void resizeBackBuffer( BcU32 Width, BcU32 Height ) = 0;

	/**
	 *	Begin frame.
	 */
	virtual void beginFrame() = 0;
	
	/**
	 * End frame.
	 */
	virtual void endFrame() = 0;

	/**
	 * Present.
	 */
	virtual void present() = 0;

	/**
	 * Requests a screenshot from the context.
	 * @param ScreenshotFunc Function to call when screenshot has been taken.
	 */
	virtual void takeScreenshot( RsScreenshotFunc ScreenshotFunc ) = 0;

	/**
	 * Create render state.
	 * @param RenderState Render state to create from.
	 */
	virtual bool createRenderState(
		class RsRenderState* RenderState ) = 0;

	/**
	 * Destroy render state.
	 * @param RenderState Render state to destroy.
	 */
	virtual bool destroyRenderState( 
		class RsRenderState* RenderState ) = 0;

	/**
	 * Create sampler state.
	 * @param SamplerState Sampler state to create from.
	 */
	virtual bool createSamplerState(
		class RsSamplerState* SamplerState ) = 0;

	/**
	 * Destroy sampler state.
	 * @param SamplerState Render state to destroy.
	 */
	virtual bool destroySamplerState( 
		class RsSamplerState* SamplerState ) = 0;

	/**
	 * Create frame buffer.
	 * @param FrameBuffer Frame buffer to create from.
	 */
	virtual bool createFrameBuffer( 
		class RsFrameBuffer* FrameBuffer ) = 0;

	/**
	 * Destroy frame buffer.
	 * @param FrameBuffer Frane buffer to destroy.
	 */
	virtual bool destroyFrameBuffer( 
		class RsFrameBuffer* FrameBuffer ) = 0;

	/**
	 * Create buffer.
	 * @param Buffer Buffer to create from.
	 */
	virtual bool createBuffer( 
		class RsBuffer* Buffer ) = 0;

	/**
	 * Destroy buffer.
	 * @param Buffer Buffer to destroy.
	 */
	virtual bool destroyBuffer( 
		class RsBuffer* Buffer ) = 0;

	/**
	 * Update buffer.
	 * @param Buffer to update.
	 * @param Offset to update.
	 * @param Size to update.
	 * @param Flags for update.
	 * @param UpdateFunc to call.
	 */
	virtual bool updateBuffer( 
		class RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsResourceUpdateFlags Flags,
		RsBufferUpdateFunc UpdateFunc ) = 0;

	/**
	 * Create texture.
	 * @param Texture Texture to create from.
	 */
	virtual bool createTexture( 
		class RsTexture* Texture ) = 0;

	/**
	 * Destroy texture.
	 * @param Texture Texture to destroy.
	 */
	virtual bool destroyTexture( 
		class RsTexture* Texture ) = 0;

	/**
	 * Update texture
	 * @param Texture to update.
	 */
	virtual bool updateTexture( 
		class RsTexture* Texture,
		const struct RsTextureSlice& Slice,
		RsResourceUpdateFlags Flags,
		RsTextureUpdateFunc UpdateFunc ) = 0;

	/**
	 * Create shader.
	 * @param Shader Shader to create from.
	 */
	virtual bool createShader(
		class RsShader* Shader ) = 0;

	/**
	 * Destroy shader.
	 * @param Shader Shader to destroy.
	 */
	virtual bool destroyShader(
		class RsShader* Shader ) = 0;

	/**
	 * Create program.
	 * @param Program Program to create from.
	 */
	virtual bool createProgram(
		class RsProgram* Program ) = 0;

	/**
	 * Destroy program.
	 * @param Program Program to destory.
	 */
	virtual bool destroyProgram(
		class RsProgram* Program ) = 0;

	/**
	 * Create program binding.
	 * @param ProgramBinding Program binding to create from.
	 */
	virtual bool createProgramBinding(
		class RsProgramBinding* ProgramBinding ) = 0;

	/**
	 * Destroy program binding.
	 * @param ProgramBinding Program binding to destroy.
	 */
	virtual bool destroyProgramBinding( 
		class RsProgramBinding* ProgramBinding ) = 0;

	/**
	 * Create geometry binding.
	 * @param GeometryBinding GeometryBinding to create from.
	 */
	virtual bool createGeometryBinding( 
		class RsGeometryBinding* GeometryBinding ) = 0;

	/**
	 * Create geometry binding.
	 * @param GeometryBinding GeometryBinding to create from.
	 */
	virtual bool destroyGeometryBinding( 
		class RsGeometryBinding* GeometryBinding ) = 0;

	/**
	 * Create vertex declaration.
	 * @param VertexDeclaration VertexDeclaration to create from.
	 */
	virtual bool createVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration ) = 0;

	/**
	 * Destroy VertexDeclaration.
	 * @param VertexDeclaration VertexDeclaration to destory.
	 */
	virtual bool destroyVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration  ) = 0;

	/**
	 * Create query heap.
	 * @param QueryHeap Query heap to create from.
	 */
	virtual bool createQueryHeap(
		class RsQueryHeap* QueryHeap ) = 0;

	/**
	 * Destroy query heap.
	 * @param QueryHeap Query heap to destroy.
	 */
	virtual bool destroyQueryHeap( 
		class RsQueryHeap* QueryHeap ) = 0;

	/**
	 * Clear.
	 * @param FrameBuffer Frame buffer we wish to render to. nullptr for backbuffer.
	 * @param Colour Colour to clear to.
	 * @param EnableClearColour Should we clear colour target?
	 * @param EnableClearDepth Should we clear depth target?
	 * @param EnableClearStencil Should we clear stencil target?
	 */
	virtual void clear( 
		const RsFrameBuffer* FrameBuffer,
		const RsColour& Colour,
		BcBool EnableClearColour,
		BcBool EnableClearDepth,
		BcBool EnableClearStencil ) = 0;
	
	/**
	 * Draw primitives.
	 * @param GeometryBinding Geometry to bind for draw.
	 * @param ProgramBinding Program + resource to bind for draw.
	 * @param RenderState Render state to draw with.
	 * @param FrameBufer Frame buffer we wish to render to. nullptr for backbuffer.
	 * @param Viewport Viewport to render to. nullptr for full @a FrameBuffer.
	 * @param ScissorRect Scissor rect to set. nullptr for full @a Viewport.
	 * @param PrimitiveType Type of primitive to draw.
	 * @param VertexOffset How many vertices to start rendering in from.
	 * @param NoofVertices How many vertices to draw.
	 * @param FirstInstance First instance to draw.
	 * @param NoofInstances Number of instances to draw.
	 */
	virtual void drawPrimitives( 
		const class RsGeometryBinding* GeometryBinding, 
		const class RsProgramBinding* ProgramBinding, 
		const class RsRenderState* RenderState,
		const class RsFrameBuffer* FrameBuffer,
		const struct RsViewport* Viewport,
		const struct RsScissorRect* ScissorRect,
		RsTopologyType PrimitiveType, 
		BcU32 VertexOffset, BcU32 NoofVertices,
		BcU32 FirstInstance, BcU32 NoofInstances ) { BcBreakpoint; };

	/**
	 * Draw indexed primitives.
	 * @param GeometryBinding Geometry to bind for draw.
	 * @param ProgramBinding Program + resource to bind for draw.
	 * @param RenderState Render state to draw with.
	 * @param FrameBufer Frame buffer we wish to render to. nullptr for backbuffer.
	 * @param Viewport Viewport to render to. nullptr for full @a FrameBuffer.
	 * @param ScissorRect Scissor rect to set. nullptr for full @a Viewport.
	 * @param PrimitiveType Type of primitive to draw.
	 * @param IndexOffset How many indices to start rendering in from.
	 * @param NoofIndices How many indices to draw.
	 * @param VertexOffset Base vertex offset to index from.
	 * @param FirstInstance First instance to draw.
	 * @param NoofInstances Number of instances to draw.
	 */
	virtual void drawIndexedPrimitives( 
		const class RsGeometryBinding* GeometryBinding, 
		const class RsProgramBinding* ProgramBinding, 
		const class RsRenderState* RenderState,
		const class RsFrameBuffer* FrameBuffer,
		const struct RsViewport* Viewport,
		const struct RsScissorRect* ScissorRect,
		RsTopologyType PrimitiveType, 
		BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset,
		BcU32 FirstInstance, BcU32 NoofInstances ) { BcBreakpoint; };

	/**
	 * Copy texture to another texture.
	 * @param SourceTexture Texture to copy from.
	 * @param DestTexture Texture to copy to.
	 * @pre @a SourceTexture must be a TEX2D (Support for 1D, 3D, CUBE to be added later).
	 * @pre @a DestTexture must be a TEX2D (Support for 1D, 3D, CUBE to be added later).
	 * @pre @a SourceTexture width must match @a DestTexture width.
	 * @pre @a SourceTexture height must match @a DestTexture height.
	 * @pre @a SourceTexture levels must match @a DestTexture levels.
	 * @pre @a SourceTexture format must match @a DestTexture format.
	 */
	virtual void copyTexture( RsTexture* SourceTexture, RsTexture* DestTexture ) { BcBreakpoint; };

	/**
	 * Dispatch compute.
	 * @param ProgramBinding Program binding.
	 * @param XGroups Number of X groups to dispatch.
	 * @param YGroups Number of Y groups to dispatch.
	 * @param ZGroups Number of Z groups to dispatch.
	 */
	virtual void dispatchCompute( class RsProgramBinding* ProgramBinding, BcU32 XGroups, BcU32 YGroups, BcU32 ZGroups ) { BcBreakpoint; };

	/**
	 * Begin query.
	 * @param QueryHeap Query heap to use.
	 * @param Idx Index of query within query heap.
	 */
	virtual void beginQuery( class RsQueryHeap* QueryHeap, size_t Idx ) { BcBreakpoint; }

	/**
	 * End query.
	 * @param QueryHeap Query heap to use.
	 * @param Idx Index of query within query heap.
	 */
	virtual void endQuery( class RsQueryHeap* QueryHeap, size_t Idx ) { BcBreakpoint; }

	/**
	 * Is query result availible?
	 * @param QueryHeap Query heap to use.
	 * @param Idx Index of query to check.
	 * @return Is query result availible?
	 */
	virtual bool isQueryResultAvailible( class RsQueryHeap* QueryHeap, size_t Idx ) { BcBreakpoint; return false; }

	/**
	 * Resolve queries.
	 * @param QueryHeap Query heap to use.
	 * @param Idx Index of query to begin resolving.
	 * @param NoofQueries Number of queries to resolve.
	 * @param OutData Pointer to output data.
	 */
	virtual void resolveQueries( class RsQueryHeap* QueryHeap, size_t Offset, size_t NoofQueries, BcU64* OutData ) { BcBreakpoint; }

};

#endif
