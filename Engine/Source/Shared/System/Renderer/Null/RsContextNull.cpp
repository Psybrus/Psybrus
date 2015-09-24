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
// beginFrame
void RsContextNull::beginFrame( BcU32 Width, BcU32 Height )
{
	Width_ = Width;
	Height_ = Height;
}

//////////////////////////////////////////////////////////////////////////
// endFrame
void RsContextNull::endFrame()
{
}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
void RsContextNull::takeScreenshot( RsScreenshotFunc ScreenshotFunc )
{
}

//////////////////////////////////////////////////////////////////////////
// setViewport
void RsContextNull::setViewport( const class RsViewport& )
{
}

//////////////////////////////////////////////////////////////////////////
// create
void RsContextNull::create()
{
	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();
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
// setDefaultState
void RsContextNull::setDefaultState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsContextNull::invalidateRenderState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsContextNull::invalidateTextureState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextNull::setRenderState( RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextNull::setSamplerState( BcU32 Handle, class RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextNull::setSamplerState( BcU32 Handle, const RsTextureParams& Params, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void RsContextNull::setTexture( BcU32 Handle, RsTexture* pTexture, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setProgram
void RsContextNull::setProgram( class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setIndexBuffer
void RsContextNull::setIndexBuffer( class RsBuffer* IndexBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setVertexBuffer
void RsContextNull::setVertexBuffer( 
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
void RsContextNull::setUniformBuffer( 
	BcU32 Handle, 
	class RsBuffer* UniformBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Handle );
	BcUnusedVar( UniformBuffer );
}

//////////////////////////////////////////////////////////////////////////
// setVertexDeclaration
void RsContextNull::setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( VertexDeclaration );
}

//////////////////////////////////////////////////////////////////////////
// setFrameBuffer
void RsContextNull::setFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextNull::clear( 
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
void RsContextNull::drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( PrimitiveType );
	BcUnusedVar( IndexOffset );
	BcUnusedVar( NoofIndices );
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextNull::drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( PrimitiveType );
	BcUnusedVar( IndexOffset );
	BcUnusedVar( NoofIndices );
	BcUnusedVar( VertexOffset );
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
// flushState
//virtual
void RsContextNull::flushState()
{
}
