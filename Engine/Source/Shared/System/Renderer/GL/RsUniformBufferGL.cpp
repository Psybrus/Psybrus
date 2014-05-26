/**************************************************************************
*
* File:		RsUniformBufferGL.cpp
* Author:	Neil Richardson
* Ver/Date:	
* Description:
*
*
*
*
*
**************************************************************************/

#include "System/Renderer/GL/RsUniformBufferGL.h"

#include "System/Renderer/RsCore.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsUniformBufferGL::RsUniformBufferGL( RsContext* pContext, const RsUniformBufferDesc& Desc, void* pBufferData ):
	RsUniformBuffer( pContext )
{
	// Setup base buffer.
	Type_ = GL_UNIFORM_BUFFER;
	Usage_ = pBufferData != NULL ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW; // TODO: Take from a user parameter.

	// Setup stride and descriptor.
	Desc_ = Desc;
	pData_ = pBufferData;
	DataSize_ = Desc_.BufferSize_;

	// Create data if we need to.
	if( pData_ == NULL )
	{
		pData_ = new BcU8[ DataSize_ ];
		DeleteData_ = BcTrue;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsUniformBufferGL::~RsUniformBufferGL()
{

}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsUniformBufferGL::create()
{
	// Generate buffers.
	GLuint Handle;
	glGenBuffers( 1, &Handle );
	setHandle( Handle );

	if( Handle != 0 )
	{
		UpdateSyncFence_.increment();
		update();

		// Destroy if there is a failure.
		if ( glGetError() != GL_NO_ERROR )
		{
			destroy();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsUniformBufferGL::update()
{
	GLuint Handle = getHandle< GLuint >();

	if( pData_ != NULL )
	{
		// Bind and push data into buffer.
		glBindBuffer( Type_, Handle );
		glBufferData( Type_, DataSize_, pData_, Usage_ );

		// Catch error.
		RsGLCatchError();

		glBindBuffer( Type_, 0 );

		UpdateSyncFence_.decrement();
	}
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsUniformBufferGL::destroy()
{
	GLuint Handle = getHandle< GLuint >();

	if( Handle != 0 )
	{
		glDeleteBuffers( 1, &Handle );
		setHandle< GLuint >( 0 );
	}
}



////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void* RsUniformBufferGL::lock()
{
	wait();
	return pData_;
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsUniformBufferGL::unlock()
{
	UpdateSyncFence_.increment();
	RsCore::pImpl()->updateResource( this );
}