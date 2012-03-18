/**************************************************************************
*
* File:		RsIndexBufferGL.cpp
* Author:	Neil Richardson
* Ver/Date:	25/02/11
* Description:
*
*
*
*
*
**************************************************************************/

#include "System/Renderer/GL/RsIndexBufferGL.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsIndexBufferGL::RsIndexBufferGL( BcU32 NoofIndices, void* pIndexData )
{
	// Setup base buffer.
	Type_ = GL_ELEMENT_ARRAY_BUFFER;
	Usage_ = pIndexData != NULL ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW; // TODO: Take from a user parameter.

	// Setup stride and descriptor.
	NoofIndices_ = NoofIndices;
	pData_ = pIndexData;
	DataSize_ = NoofIndices_ * sizeof( BcU16 );

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
RsIndexBufferGL::~RsIndexBufferGL()
{

}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsIndexBufferGL::create()
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
void RsIndexBufferGL::update()
{
	GLuint Handle = getHandle< GLuint >();

	if( pData_ != NULL )
	{
		// Bind and push data into buffer.
		glBindBuffer( Type_, Handle );
		glBufferData( Type_, DataSize_, pData_, Usage_ );

		// Catch error.
		RsGLCatchError;

		glBindBuffer( Type_, 0 );

		UpdateSyncFence_.decrement();
	}
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsIndexBufferGL::destroy()
{
	GLuint Handle = getHandle< GLuint >();

	if( Handle != 0 )
	{
		glDeleteBuffers( 1, &Handle );
		setHandle< GLuint >( 0 );
	}
}

////////////////////////////////////////////////////////////////////////////////
// bind
void RsIndexBufferGL::bind()
{
	GLuint Handle = getHandle< GLuint >();
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, Handle );

	// Catch error.
	RsGLCatchError;
}
