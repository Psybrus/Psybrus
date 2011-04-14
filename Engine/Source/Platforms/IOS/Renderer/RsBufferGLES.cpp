/**************************************************************************
*
* File:		RsBufferGLES.cpp
* Author:	Neil Richardson 
* Ver/Date:	25/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsBufferGLES.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsBufferGLES::RsBufferGLES():
	Type_( 0 ),
	Usage_( 0 )
{
	
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsBufferGLES::~RsBufferGLES()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsBufferGLES::create()
{
	// Generate buffers.
	glGenBuffers( 1, &Handle_ );
	
	if( Handle_ != 0 )
	{
		// Bind and push data into buffer.
		glBindBuffer( Type_, Handle_ );
		glBufferData( Type_, DataSize_, pData_, Usage_ );
		glBindBuffer( Type_, 0 );
		
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
void RsBufferGLES::update()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsBufferGLES::destroy()
{
	if( Handle_ != 0 )
	{
		glDeleteBuffers( 1, &Handle_ );
		Handle_ = 0;
	}
}

