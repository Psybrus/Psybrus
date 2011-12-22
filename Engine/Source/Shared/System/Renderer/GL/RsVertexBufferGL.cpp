/**************************************************************************
*
* File:		RsVertexBufferGL.cpp
* Author:	Neil Richardson 
* Ver/Date:	25/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsVertexBufferGL.h"

#include "RsCore.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsVertexBufferGL::RsVertexBufferGL( BcU32 Descriptor, BcU32 NoofVertices, void* pVertexData )
{
	// Setup base buffer.
	Type_ = GL_ARRAY_BUFFER;
	Usage_ = pVertexData != NULL ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW; // TODO: Take from a user parameter.
	
	// Setup stride and descriptor.
	Descriptor_ = Descriptor;
	Stride_ = RsVertexDeclSize( Descriptor );
	NoofVertices_ = NoofVertices;
	NoofUpdateVertices_ = NoofVertices;
	pData_ = pVertexData;
	DataSize_ = Stride_ * NoofVertices;
	
	// Create data if we need to.
	if( pData_ == NULL )
	{
		pData_ = new BcU8[ Stride_ * NoofVertices_ ];
		DeleteData_ = BcTrue;
	}

	Created_ = BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsVertexBufferGL::~RsVertexBufferGL()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void* RsVertexBufferGL::lock()
{
	Lock_.lock();
	return pData_;
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsVertexBufferGL::unlock()
{
	Lock_.unlock();
	RsCore::pImpl()->updateResource( this );
}

////////////////////////////////////////////////////////////////////////////////
// setNoofUpdateVertices
//virtual
void RsVertexBufferGL::setNoofUpdateVertices( BcU32 NoofVertices )
{
	NoofUpdateVertices_ = BcMin( NoofVertices, NoofVertices_ );
}

////////////////////////////////////////////////////////////////////////////////
// getNoofVertices
//virtual
BcU32 RsVertexBufferGL::getNoofVertices()
{
	return NoofVertices_;
}

////////////////////////////////////////////////////////////////////////////////
// getVertexFormat
//virtual
BcU32 RsVertexBufferGL::getVertexFormat()
{
	return Descriptor_;
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsVertexBufferGL::create()
{
	// Generate buffers.
	GLuint Handle;
	glGenBuffers( 1, &Handle );
	setHandle( Handle );
	
	if( Handle != 0 )
	{
		update();
		
		// Destroy if there is a failure.
		if ( glGetError() != GL_NO_ERROR )
		{
			destroy();
		}
		else
		{
			Created_ = BcTrue;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsVertexBufferGL::update()
{
	GLuint Handle = getHandle< GLuint >();

	if( pData_ != NULL )
	{
		// Bind buffer.
		glBindBuffer( Type_, Handle );
	
		// Lock, buffer, and unlock.
		Lock_.lock();
		if( Created_ )
		{
			glBufferSubData( Type_, 0, NoofUpdateVertices_ * Stride_, pData_ );
		}
		else
		{
			glBufferData( Type_, NoofVertices_ * Stride_, pData_, Usage_ );
		}
		Lock_.unlock();
	
		// Unbind buffer.
		glBindBuffer( Type_, 0 );
	}
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsVertexBufferGL::destroy()
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
void RsVertexBufferGL::bind()
{
	const BcU32 VertexDecl = Descriptor_;
	const BcU32 Stride = Stride_;
	char* Offset = NULL;

	// Bind vertex buffer.
	GLuint Handle = getHandle< GLuint >();
	glBindBuffer( GL_ARRAY_BUFFER, Handle );

	// NOTE: This should be optimised to use a lookup table or similar.

	// Position.
	if( VertexDecl & rsVDF_POSITION_XY )
	{
		glEnableVertexAttribArray( rsVC_POSITION );
		glVertexAttribPointer( rsVC_POSITION, 2, GL_FLOAT, 0, Stride, Offset );
		Offset += 2 * sizeof( BcF32 );
	}
	else if( VertexDecl & rsVDF_POSITION_XYZ )
	{
		glEnableVertexAttribArray( rsVC_POSITION );
		glVertexAttribPointer( rsVC_POSITION, 3, GL_FLOAT, 0, Stride, Offset );
		Offset += 3 * sizeof( BcF32 );
	}
	else if( VertexDecl & rsVDF_POSITION_XYZW )
	{
		glEnableVertexAttribArray( rsVC_POSITION );
		glVertexAttribPointer( rsVC_POSITION, 4, GL_FLOAT, 0, Stride, Offset );
		Offset += 4 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_POSITION );
	}
	
	// Normal.
	if( VertexDecl & rsVDF_NORMAL_XYZ )
	{
		glEnableVertexAttribArray( rsVC_NORMAL );
		glVertexAttribPointer( rsVC_NORMAL, 3, GL_FLOAT, 0, Stride, Offset );
		Offset += 3 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_NORMAL );
	}
	
	// Tangent.
	if( VertexDecl & rsVDF_TANGENT_XYZ )
	{
		glEnableVertexAttribArray( rsVC_TANGENT );
		glVertexAttribPointer( rsVC_TANGENT, 3, GL_FLOAT, 0, Stride, Offset );
		Offset += 3 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_TANGENT );
	}
	
	// TexCoord0.
	if( VertexDecl & rsVDF_TEXCOORD_UV0 )
	{
		glEnableVertexAttribArray( rsVC_TEXCOORD0 );
		glVertexAttribPointer( rsVC_TEXCOORD0, 2, GL_FLOAT, 0, Stride, Offset );
		Offset += 2 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_TEXCOORD0 );
	}
	
	// TexCoord1.
	if( VertexDecl & rsVDF_TEXCOORD_UV1 )
	{
		glEnableVertexAttribArray( rsVC_TEXCOORD1 );
		glVertexAttribPointer( rsVC_TEXCOORD1, 2, GL_FLOAT, 0, Stride, Offset );
		Offset += 2 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_TEXCOORD1 );
	}
	
	// TexCoord2.
	if( VertexDecl & rsVDF_TEXCOORD_UV2 )
	{
		glEnableVertexAttribArray( rsVC_TEXCOORD2 );
		glVertexAttribPointer( rsVC_TEXCOORD2, 2, GL_FLOAT, 0, Stride, Offset );
		Offset += 2 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_TEXCOORD2 );
	}
	
	// TexCoord3.
	if( VertexDecl & rsVDF_TEXCOORD_UV3 )
	{
		glEnableVertexAttribArray( rsVC_TEXCOORD3 );
		glVertexAttribPointer( rsVC_TEXCOORD3, 2, GL_FLOAT, 0, Stride, Offset );
		Offset += 2 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_TEXCOORD3 );
	}
	
	// Colour.
	if( VertexDecl & rsVDF_COLOUR_RGBA8 )
	{
		glEnableVertexAttribArray( rsVC_COLOUR );
		glVertexAttribPointer( rsVC_COLOUR, 4, GL_UNSIGNED_BYTE, 1, Stride, Offset );
		Offset += 4 * sizeof( BcU8 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_COLOUR );
	}	
		
	// Catch error.
	RsGLCatchError;
}

