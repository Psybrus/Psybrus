/**************************************************************************
*
* File:		RsIndexBufferGLES.cpp
* Author:	Neil Richardson 
* Ver/Date:	25/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsIndexBufferGLES.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsIndexBufferGLES::RsIndexBufferGLES( BcU32 NoofIndices, void* pIndexData )
{
	// Setup base buffer.
	Type_ = GL_ELEMENT_ARRAY_BUFFER;
	Usage_ = pIndexData != NULL ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW; // TODO: Take from a user parameter.
	
	// Setup stride and descriptor.
	NoofIndices_ = NoofIndices;
	pData_ = pIndexData;
	
	// Create data if we need to.
	if( pData_ == NULL )
	{
		pData_ = new BcU8[ NoofIndices_ * sizeof( BcU16 ) ];
		DeleteData_ = BcTrue;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsIndexBufferGLES::~RsIndexBufferGLES()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsIndexBufferGLES::create()
{
	RsBufferGLES::create();
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsIndexBufferGLES::update()
{
	RsBufferGLES::update();
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsIndexBufferGLES::destroy()
{
	RsBufferGLES::destroy();
}

////////////////////////////////////////////////////////////////////////////////
// bind
void RsIndexBufferGLES::bind()
{
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, Handle_ );
}