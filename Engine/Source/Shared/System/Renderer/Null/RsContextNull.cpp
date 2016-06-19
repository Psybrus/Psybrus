/**************************************************************************
*
* File:		RsContextNull.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/Null/RsContextNull.h"

#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsShader.h"
#include "System/Renderer/RsTexture.h"
#include "System/Renderer/RsVertexDeclaration.h"
#include "System/Renderer/RsViewport.h"

#include "Base/BcMath.h"

#include "System/Os/OsClient.h"

#include "Import/Img/Img.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsContextNull::RsContextNull( OsClient* pClient, RsContextNull* pParent ):
	RsContext( pParent ),
	pParent_( pParent ),
	pClient_( pClient ),
	Width_( 0 ),
	Height_( 0 ),
	OwningThread_( BcErrorCode )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsContextNull::~RsContextNull()
{

}

//////////////////////////////////////////////////////////////////////////
// getClient
//virtual
OsClient* RsContextNull::getClient() const
{
	return pClient_;
}

//////////////////////////////////////////////////////////////////////////
// getFeatures
//virtual
const RsFeatures& RsContextNull::getFeatures() const
{
	return Features_;
}

//////////////////////////////////////////////////////////////////////////
// isShaderCodeTypeSupported
//virtual
BcBool RsContextNull::isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// maxShaderCodeType
//virtual
RsShaderCodeType RsContextNull::maxShaderCodeType( RsShaderCodeType CodeType ) const
{
	return RsShaderCodeType::MAX;
}

//////////////////////////////////////////////////////////////////////////
// getWidth
//virtual
BcU32 RsContextNull::getWidth() const
{
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 RsContextNull::getHeight() const
{
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// getBackBuffer
class RsFrameBuffer* RsContextNull::getBackBuffer() const
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// resizeBackBuffer
void RsContextNull::resizeBackBuffer( BcU32 Width, BcU32 Height )
{
	Width_ = Width;
	Height_ = Height;
}

//////////////////////////////////////////////////////////////////////////
// beginFrame
void RsContextNull::beginFrame()
{
}

//////////////////////////////////////////////////////////////////////////
// endFrame
void RsContextNull::endFrame()
{
}

//////////////////////////////////////////////////////////////////////////
// present
void RsContextNull::present()
{
}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
void RsContextNull::takeScreenshot( RsScreenshotFunc ScreenshotFunc )
{
}

//////////////////////////////////////////////////////////////////////////
// create
void RsContextNull::create()
{
	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();

	// Setup formats to all be supported.
	Features_.Texture2D_ = true;
	Features_.TextureFormat_.fill( true );
	Features_.RenderTargetFormat_.fill( true );
	Features_.DepthStencilTargetFormat_.fill( true );
}

//////////////////////////////////////////////////////////////////////////
// update
void RsContextNull::update()
{
}

//////////////////////////////////////////////////////////////////////////
// destroy
void RsContextNull::destroy()
{
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextNull::clear( 
	const RsFrameBuffer* FrameBuffer, 
	const RsColour& Colour,
	BcBool EnableClearColour,
	BcBool EnableClearDepth,
	BcBool EnableClearStencil )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Colour );
	BcUnusedVar( EnableClearColour );
	BcUnusedVar( EnableClearDepth );
	BcUnusedVar( EnableClearStencil );
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextNull::drawPrimitives(
	const RsGeometryBinding* GeometryBinding, 
	const RsProgramBinding* ProgramBinding, 
	const RsRenderState* RenderState,
	const RsFrameBuffer* FrameBuffer, 
	const RsViewport* Viewport,
	const RsScissorRect* ScissorRect,
	RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices,
	BcU32 FirstInstance, BcU32 NoofInstances )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( PrimitiveType );
	BcUnusedVar( IndexOffset );
	BcUnusedVar( NoofIndices );
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextNull::drawIndexedPrimitives( 
	const RsGeometryBinding* GeometryBinding, 
	const RsProgramBinding* ProgramBinding, 
	const RsRenderState* RenderState,
	const RsFrameBuffer* FrameBuffer, 
	const RsViewport* Viewport,
	const RsScissorRect* ScissorRect,
	RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset,
	BcU32 FirstInstance, BcU32 NoofInstances )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( PrimitiveType );
	BcUnusedVar( IndexOffset );
	BcUnusedVar( NoofIndices );
	BcUnusedVar( VertexOffset );
}

//////////////////////////////////////////////////////////////////////////
// createRenderState
void RsContextNull::copyTexture( RsTexture* SourceTexture, RsTexture* DestTexture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( SourceTexture );
	BcUnusedVar( DestTexture );
}

//////////////////////////////////////////////////////////////////////////
// createRenderState
void RsContextNull::dispatchCompute( class RsProgramBinding* ProgramBinding, BcU32 XGroups, BcU32 YGroups, BcU32 ZGroups )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( ProgramBinding );
	BcUnusedVar( XGroups );
	BcUnusedVar( YGroups );
	BcUnusedVar( ZGroups );
}

//////////////////////////////////////////////////////////////////////////
// createRenderState
void RsContextNull::beginQuery( class RsQueryHeap* QueryHeap, size_t Idx )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( QueryHeap );
	BcUnusedVar( Idx );
}

//////////////////////////////////////////////////////////////////////////
// createRenderState
void RsContextNull::endQuery( class RsQueryHeap* QueryHeap, size_t Idx )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( QueryHeap );
	BcUnusedVar( Idx );
}

//////////////////////////////////////////////////////////////////////////
// isQueryResultAvailible
bool RsContextNull::isQueryResultAvailible( class RsQueryHeap* QueryHeap, size_t Idx )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( QueryHeap );
	BcUnusedVar( Idx );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// resolveQueries
void RsContextNull::resolveQueries( class RsQueryHeap* QueryHeap, size_t Offset, size_t NoofQueries, BcU64* OutData )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( QueryHeap );
	BcUnusedVar( Offset );
	for( size_t Idx = 0; Idx < NoofQueries; ++Idx )
	{
		OutData[ Idx ] = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
// createRenderState
bool RsContextNull::createRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( RenderState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyRenderState
bool RsContextNull::destroyRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( RenderState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createSamplerState
bool RsContextNull::createSamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( SamplerState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroySamplerState
bool RsContextNull::destroySamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( SamplerState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createFrameBuffer
bool RsContextNull::createFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( FrameBuffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyFrameBuffer
bool RsContextNull::destroyFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( FrameBuffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createBuffer
bool RsContextNull::createBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Buffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyBuffer
bool RsContextNull::destroyBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Buffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// updateBuffer
bool RsContextNull::updateBuffer( 
	class RsBuffer* Buffer,
	BcSize Offset,
	BcSize Size,
	RsResourceUpdateFlags Flags,
	RsBufferUpdateFunc UpdateFunc )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Buffer );
	BcUnusedVar( Offset );
	BcUnusedVar( Size );
	BcUnusedVar( Flags );
	BcUnusedVar( UpdateFunc );
	std::unique_ptr< BcU8[] > TempBuffer;
	TempBuffer.reset( new BcU8[ Buffer->getDesc().SizeBytes_ ] );
	RsBufferLock Lock = { TempBuffer.get() };
	UpdateFunc( Buffer, Lock );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
bool RsContextNull::createTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Texture );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyTexture
bool RsContextNull::destroyTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Texture );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// updateTexture
bool RsContextNull::updateTexture( 
	class RsTexture* Texture,
	const struct RsTextureSlice& Slice,
	RsResourceUpdateFlags Flags,
	RsTextureUpdateFunc UpdateFunc )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Texture );
	BcUnusedVar( Slice );
	BcUnusedVar( Flags );
	BcUnusedVar( UpdateFunc );

	const auto& TextureDesc = Texture->getDesc();
	std::unique_ptr< BcU8[] > TempBuffer;
	BcU32 Width = BcMax( 1, TextureDesc.Width_ >> Slice.Level_ );
	BcU32 Height = BcMax( 1, TextureDesc.Height_ >> Slice.Level_ );
	BcU32 Depth = BcMax( 1, TextureDesc.Depth_ >> Slice.Level_ );
	BcU32 DataSize = RsTextureFormatSize( 
		TextureDesc.Format_,
		Width,
		Height,
		Depth,
		1 );

	BcU32 SlicePitch = RsTextureSlicePitch( 
		TextureDesc.Format_,
		Width,
		Height );
	BcU32 Pitch = RsTexturePitch( 
		TextureDesc.Format_,
		Width,
		Height );

	TempBuffer.reset( new BcU8[ DataSize ] );
	RsTextureLock Lock = 
	{
		TempBuffer.get(),
		Pitch,
		SlicePitch
	};	
	UpdateFunc( Texture, Lock );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createShader
bool RsContextNull::createShader(
	class RsShader* Shader )
{
	BcUnusedVar( Shader );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyShader
bool RsContextNull::destroyShader(
	class RsShader* Shader )
{
	BcUnusedVar( Shader );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
bool RsContextNull::createProgram(
	class RsProgram* Program )
{
	BcUnusedVar( Program );
	return true;
}


//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextNull::destroyProgram(
	class RsProgram* Program )
{
	BcUnusedVar( Program );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgramBinding
bool RsContextNull::createProgramBinding( class RsProgramBinding* ProgramBinding )
{
	BcUnusedVar( ProgramBinding );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyProgramBinding
bool RsContextNull::destroyProgramBinding( class RsProgramBinding* ProgramBinding )
{
	BcUnusedVar( ProgramBinding );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createGeometryBinding
bool RsContextNull::createGeometryBinding( class RsGeometryBinding* GeometryBinding )
{
	BcUnusedVar( GeometryBinding );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyGeometryBinding
bool RsContextNull::destroyGeometryBinding( class RsGeometryBinding* GeometryBinding )
{
	BcUnusedVar( GeometryBinding );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createVertexDeclaration
bool RsContextNull::createVertexDeclaration(
	class RsVertexDeclaration* VertexDeclaration )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyVertexDeclaration
bool RsContextNull::destroyVertexDeclaration(
	class RsVertexDeclaration* VertexDeclaration  )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createQueryHeap
bool RsContextNull::createQueryHeap( class RsQueryHeap* QueryHeap )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyQueryHeap
bool RsContextNull::destroyQueryHeap( class RsQueryHeap* QueryHeap )
{
	return true;
}
