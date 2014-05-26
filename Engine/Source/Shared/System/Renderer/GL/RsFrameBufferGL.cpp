/**************************************************************************
*
* File:		RsFrameBufferGL.cpp
* Author:	Neil Richardson 
* Ver/Date:	16/08/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/GL/RsFrameBufferGL.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsFrameBufferGL::RsFrameBufferGL( RsContext* pContext ):
	RsResource( pContext )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsFrameBufferGL::~RsFrameBufferGL()
{
	
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsFrameBufferGL::create()
{
	// Generate buffers.
	GLuint Handle = 0;
	glGenFramebuffers( 1, &Handle );
	RsGLCatchError();

	setHandle( Handle );
	
	if( Handle != 0 )
	{
		update();
		
		// Destroy if there is a failure.
		if ( glGetError() != GL_NO_ERROR )
		{
			destroy();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsFrameBufferGL::update()
{
	
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsFrameBufferGL::destroy()
{
	GLuint Handle = getHandle< GLuint >();
	
	if( Handle != 0 )
	{
		glDeleteFramebuffers( 1, &Handle );
		setHandle< GLuint >( 0 );
	}
}
