/**************************************************************************
*
* File:		RsContextVK.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/VK/RsContextVK.h"

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
RsContextVK::RsContextVK( OsClient* pClient, RsContextVK* pParent ):
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
RsContextVK::~RsContextVK()
{

}

//////////////////////////////////////////////////////////////////////////
// getClient
//virtual
OsClient* RsContextVK::getClient() const
{
	return pClient_;
}

//////////////////////////////////////////////////////////////////////////
// getFeatures
//virtual
const RsFeatures& RsContextVK::getFeatures() const
{
	return Features_;
}

//////////////////////////////////////////////////////////////////////////
// isShaderCodeTypeSupported
//virtual
BcBool RsContextVK::isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// maxShaderCodeType
//virtual
RsShaderCodeType RsContextVK::maxShaderCodeType( RsShaderCodeType CodeType ) const
{
	return RsShaderCodeType::MAX;
}

//////////////////////////////////////////////////////////////////////////
// getWidth
//virtual
BcU32 RsContextVK::getWidth() const
{
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 RsContextVK::getHeight() const
{
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// beginFrame
void RsContextVK::beginFrame( BcU32 Width, BcU32 Height )
{
	Width_ = Width;
	Height_ = Height;
}

//////////////////////////////////////////////////////////////////////////
// endFrame
void RsContextVK::endFrame()
{
}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
void RsContextVK::takeScreenshot( RsScreenshotFunc ScreenshotFunc )
{
}

//////////////////////////////////////////////////////////////////////////
// setViewport
void RsContextVK::setViewport( const class RsViewport& )
{
}

//////////////////////////////////////////////////////////////////////////
// create
void RsContextVK::create()
{
	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();
}

//////////////////////////////////////////////////////////////////////////
// update
void RsContextVK::update()
{
}

//////////////////////////////////////////////////////////////////////////
// destroy
void RsContextVK::destroy()
{
}

//////////////////////////////////////////////////////////////////////////
// setDefaultState
void RsContextVK::setDefaultState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsContextVK::invalidateRenderState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsContextVK::invalidateTextureState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextVK::setRenderState( RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextVK::setSamplerState( BcU32 Handle, class RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextVK::setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// getRenderState
BcS32 RsContextVK::getRenderState( RsRenderStateType State ) const
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextVK::setSamplerState( BcU32 Handle, const RsTextureParams& Params, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void RsContextVK::setTexture( BcU32 Handle, RsTexture* pTexture, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setProgram
void RsContextVK::setProgram( class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setIndexBuffer
void RsContextVK::setIndexBuffer( class RsBuffer* IndexBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setVertexBuffer
void RsContextVK::setVertexBuffer( 
	BcU32 StreamIdx, 
	class RsBuffer* VertexBuffer,
	BcU32 Stride )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( StreamIdx );
	BcUnusedVar( VertexBuffer );
	BcUnusedVar( Stride );
}

//////////////////////////////////////////////////////////////////////////
// setUniformBuffer
void RsContextVK::setUniformBuffer( 
	BcU32 Handle, 
	class RsBuffer* UniformBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Handle );
	BcUnusedVar( UniformBuffer );
}

//////////////////////////////////////////////////////////////////////////
// setVertexDeclaration
void RsContextVK::setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( VertexDeclaration );
}

//////////////////////////////////////////////////////////////////////////
// setFrameBuffer
void RsContextVK::setFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextVK::clear( 
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
void RsContextVK::drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( PrimitiveType );
	BcUnusedVar( IndexOffset );
	BcUnusedVar( NoofIndices );
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextVK::drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( PrimitiveType );
	BcUnusedVar( IndexOffset );
	BcUnusedVar( NoofIndices );
	BcUnusedVar( VertexOffset );
}

//////////////////////////////////////////////////////////////////////////
// createRenderState
bool RsContextVK::createRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( RenderState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyRenderState
bool RsContextVK::destroyRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( RenderState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createSamplerState
bool RsContextVK::createSamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( SamplerState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroySamplerState
bool RsContextVK::destroySamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( SamplerState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createFrameBuffer
bool RsContextVK::createFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( FrameBuffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyFrameBuffer
bool RsContextVK::destroyFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( FrameBuffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createBuffer
bool RsContextVK::createBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Buffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyBuffer
bool RsContextVK::destroyBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Buffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// updateBuffer
bool RsContextVK::updateBuffer( 
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
bool RsContextVK::createTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Texture );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyTexture
bool RsContextVK::destroyTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Texture );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// updateTexture
bool RsContextVK::updateTexture( 
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
	TempBuffer.reset( new BcU8[ DataSize ] );
	RsTextureLock Lock = 
	{
		TempBuffer.get(),
		TextureDesc.Width_,
		TextureDesc.Width_ * TextureDesc.Height_
	};	
	UpdateFunc( Texture, Lock );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createShader
bool RsContextVK::createShader(
	class RsShader* Shader )
{
	BcUnusedVar( Shader );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyShader
bool RsContextVK::destroyShader(
	class RsShader* Shader )
{
	BcUnusedVar( Shader );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
bool RsContextVK::createProgram(
	class RsProgram* Program )
{
	BcUnusedVar( Program );
	return true;
}


//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextVK::destroyProgram(
	class RsProgram* Program )
{
	BcUnusedVar( Program );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createVertexDeclaration
bool RsContextVK::createVertexDeclaration(
	class RsVertexDeclaration* VertexDeclaration )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyVertexDeclaration
bool RsContextVK::destroyVertexDeclaration(
	class RsVertexDeclaration* VertexDeclaration  )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// flushState
//virtual
void RsContextVK::flushState()
{
}
