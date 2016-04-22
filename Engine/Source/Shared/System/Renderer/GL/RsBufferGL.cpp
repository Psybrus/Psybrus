#include "System/Renderer/GL/RsBufferGL.h"
#include "System/Renderer/GL/RsContextGL.h"
#include "System/Renderer/GL/RsUtilsGL.h"
#include "System/Renderer/RsBuffer.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsBufferGL::RsBufferGL( RsBuffer* Parent, const RsOpenGLVersion& Version ):
	Parent_( Parent )
{
	Parent_->setHandle( this );
	const auto& BufferDesc = Parent_->getDesc();
	auto ContextGL = static_cast< RsContextGL* >( Parent_->getContext() );

	BcAssert( BufferDesc.SizeBytes_ > 0 );

	// Get buffer type for GL.
	auto TypeGL = RsUtilsGL::GetBufferType( BufferDesc.BindFlags_ );

	// Get usage flags for GL.
	GLuint UsageFlagsGL = 0;
	
	// Data update frequencies.
	if( ( BufferDesc.Flags_ & RsResourceCreationFlags::STATIC ) != RsResourceCreationFlags::NONE )
	{
		UsageFlagsGL |= GL_STATIC_DRAW;
	}
	else if( ( BufferDesc.Flags_ & RsResourceCreationFlags::DYNAMIC ) != RsResourceCreationFlags::NONE )
	{
		UsageFlagsGL |= GL_DYNAMIC_DRAW;
	}
	else if( ( BufferDesc.Flags_ & RsResourceCreationFlags::STREAM ) != RsResourceCreationFlags::NONE )
	{
		UsageFlagsGL |= GL_STREAM_DRAW;
	}

	// Always back uniform buffers in main memory, or if flags are DYNAMIC or STREAM.
	// Uniforms are small buffers that change frequently, and regardless of flags specified,
	// may need to be implemented as regular uniforms in GL (GL ES especially so).
	// Buffers with DYNAMIC or STREAM want to avoid allocation on the upload.
	// Later this should be replaced with a ring buffer.
	const bool IsUniformBuffer = ( BufferDesc.BindFlags_ & RsResourceBindFlags::UNIFORM_BUFFER ) != RsResourceBindFlags::NONE;
	const bool BufferInMainMemory = IsUniformBuffer ||
		( BufferDesc.Flags_ & RsResourceCreationFlags::DYNAMIC ) != RsResourceCreationFlags::NONE ||
		( BufferDesc.Flags_ & RsResourceCreationFlags::STREAM ) != RsResourceCreationFlags::NONE;
	if( BufferInMainMemory )
	{
		// Buffer is in main memory.
		BufferData_.reset( new BcU8[ BufferDesc.SizeBytes_ ] );
	}

	// Determine which kind of buffer to create.
	if( !IsUniformBuffer ||
		Version.SupportUniformBuffers_ )
	{
		// Generate buffer.
		GL( GenBuffers( 1, &Handle_ ) );

#if !defined( RENDER_USE_GLES ) && !PSY_PRODUCTION
		if( GLEW_KHR_debug )
		{
			glObjectLabel( GL_BUFFER, Handle_, BcStrLength( Parent->getDebugName() ), Parent->getDebugName() );
		}
#endif

		// Attempt to update it.
		if( Handle_ != 0 )
		{
			ContextGL->bindBuffer( TypeGL, Parent_ );
			GL( BufferData( TypeGL, BufferDesc.SizeBytes_, nullptr, UsageFlagsGL ) );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsBufferGL::~RsBufferGL()
{
	if( Handle_ != 0 )
	{
		GL( DeleteBuffers( 1, &Handle_ ) );
		Handle_ = 0;
	}
}
