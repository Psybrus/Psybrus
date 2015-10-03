#include "System/Renderer/GL/RsBufferGL.h"
#include "System/Renderer/GL/RsUtilsGL.h"
#include "System/Renderer/RsBuffer.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsBufferGL::RsBufferGL( RsBuffer* Parent, const RsOpenGLVersion& Version ):
	Parent_( Parent )
{
	const auto& BufferDesc = Parent_->getDesc();

	BcAssert( BufferDesc.SizeBytes_ > 0 );

	// Get buffer type for GL.
	auto TypeGL = RsUtilsGL::GetBufferType( BufferDesc.Type_ );

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
	const bool BufferInMainMemory = Parent_->getDesc().Type_ == RsBufferType::UNIFORM ||
		( BufferDesc.Flags_ & RsResourceCreationFlags::DYNAMIC ) != RsResourceCreationFlags::NONE ||
		( BufferDesc.Flags_ & RsResourceCreationFlags::STREAM ) != RsResourceCreationFlags::NONE;
	if( BufferInMainMemory )
	{
		// Buffer is in main memory.
		BufferData_.reset( new BcU8[ BufferDesc.SizeBytes_ ] );
	}

	// Determine which kind of buffer to create.
	if( Parent_->getDesc().Type_ != RsBufferType::UNIFORM ||
		Version.SupportUniformBuffers_ )
	{
		// Generate buffer.
		GL( GenBuffers( 1, &Handle_ ) );

		// Attempt to update it.
		if( Handle_ != 0 )
		{
			GL( BindBuffer( TypeGL, Handle_ ) );
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
