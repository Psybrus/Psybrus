/**************************************************************************
*
* File:		RsVertexBufferGLES.cpp
* Author:	Neil Richardson 
* Ver/Date:	25/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsVertexBufferGLES.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsVertexBufferGLES::RsVertexBufferGLES( BcU32 Descriptor, BcU32 NoofVertices, void* pVertexData )
{
	// Setup base buffer.
	Type_ = GL_ARRAY_BUFFER;
	Usage_ = pVertexData != NULL ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW; // TODO: Take from a user parameter.
	
	// Setup stride and descriptor.
	Descriptor_ = Descriptor;
	Stride_ = RsVertexDeclSize( Descriptor );
	NoofVertices_ = NoofVertices;
	pData_ = pVertexData;
	DataSize_ = Stride_ * NoofVertices;
	
	// Create data if we need to.
	if( pData_ == NULL )
	{
		pData_ = new BcU8[ Stride_ * NoofVertices_ ];
		DeleteData_ = BcTrue;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsVertexBufferGLES::~RsVertexBufferGLES()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsVertexBufferGLES::create()
{
	RsBufferGLES::create();
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsVertexBufferGLES::update()
{
	RsBufferGLES::update();
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsVertexBufferGLES::destroy()
{
	RsBufferGLES::destroy();
}

////////////////////////////////////////////////////////////////////////////////
// bind
void RsVertexBufferGLES::bind()
{
	const BcU32 VertexDecl = Descriptor_;
	const BcU32 Stride = Stride_;
	BcU32 Offset = 0;
	
	// Bind vertex buffer.
	glBindBuffer( GL_ARRAY_BUFFER, Handle_ );
	
	// NOTE: This should be optimised to use a lookup table or similar
	//       so we can easily.
	
	// Position.
	if( VertexDecl & rsVDF_POSITION_XY )
	{
		glVertexAttribPointer( rsVC_POSITION, 2, GL_FLOAT, 0, Stride, (char*)Offset );
		glEnableVertexAttribArray( rsVC_POSITION );
		Offset += 2 * sizeof( BcF32 );
	}
	else if( VertexDecl & rsVDF_POSITION_XYZ )
	{
		glVertexAttribPointer( rsVC_POSITION, 3, GL_FLOAT, 0, Stride, (char*)Offset );
		glEnableVertexAttribArray( rsVC_POSITION );
		Offset += 3 * sizeof( BcF32 );
	}
	else if( VertexDecl & rsVDF_POSITION_XYZW )
	{
		glVertexAttribPointer( rsVC_POSITION, 4, GL_FLOAT, 0, Stride, (char*)Offset );
		glEnableVertexAttribArray( rsVC_POSITION );
		Offset += 4 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_POSITION );
	}
	
	// Normal.
	if( VertexDecl & rsVDF_NORMAL_XYZ )
	{
		glVertexAttribPointer( rsVC_NORMAL, 3, GL_FLOAT, 0, Stride, (char*)Offset );
		glEnableVertexAttribArray( rsVC_NORMAL );
		Offset += 3 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_NORMAL );
	}
	
	// Tangent.
	if( VertexDecl & rsVDF_TANGENT_XYZ )
	{
		glVertexAttribPointer( rsVC_TANGENT, 3, GL_FLOAT, 0, Stride, (char*)Offset );
		glEnableVertexAttribArray( rsVC_TANGENT );
		Offset += 3 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_TANGENT );
	}
	
	// TexCoord0.
	if( VertexDecl & rsVDF_TEXCOORD_UV0 )
	{
		glVertexAttribPointer( rsVC_TEXCOORD0, 2, GL_FLOAT, 0, Stride, (char*)Offset );
		glEnableVertexAttribArray( rsVC_TEXCOORD0 );
		Offset += 2 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_TEXCOORD0 );
	}
	
	// TexCoord1.
	if( VertexDecl & rsVDF_TEXCOORD_UV1 )
	{
		glVertexAttribPointer( rsVC_TEXCOORD1, 2, GL_FLOAT, 0, Stride, (char*)Offset );
		glEnableVertexAttribArray( rsVC_TEXCOORD1 );
		Offset += 2 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_TEXCOORD1 );
	}
	
	// TexCoord2.
	if( VertexDecl & rsVDF_TEXCOORD_UV2 )
	{
		glVertexAttribPointer( rsVC_TEXCOORD2, 2, GL_FLOAT, 0, Stride, (char*)Offset );
		glEnableVertexAttribArray( rsVC_TEXCOORD2 );
		Offset += 2 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_TEXCOORD2 );
	}
	
	// TexCoord3.
	if( VertexDecl & rsVDF_TEXCOORD_UV3 )
	{
		glVertexAttribPointer( rsVC_TEXCOORD3, 2, GL_FLOAT, 0, Stride, (char*)Offset );
		glEnableVertexAttribArray( rsVC_TEXCOORD3 );
		Offset += 2 * sizeof( BcF32 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_TEXCOORD3 );
	}
	
	// Colour.
	if( VertexDecl & rsVDF_COLOUR_RGBA8 )
	{
		glVertexAttribPointer( rsVC_COLOUR, 4, GL_UNSIGNED_BYTE, 1, Stride, (char*)Offset );
		glEnableVertexAttribArray( rsVC_COLOUR );
		Offset += 4 * sizeof( BcU8 );
	}
	else
	{
		glDisableVertexAttribArray( rsVC_COLOUR );
	}		
}

