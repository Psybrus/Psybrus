/**************************************************************************
*
* File:		RsPrimitiveGLES.cpp
* Author:	Neil Richardson 
* Ver/Date:	4/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsPrimitiveGLES.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsPrimitiveGLES::RsPrimitiveGLES( RsVertexBufferGLES* pVertexBuffer, RsIndexBufferGLES* pIndexBuffer )
{
	pVertexBuffer_ = pVertexBuffer;
	pIndexBuffer_ = pIndexBuffer;
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsPrimitiveGLES::~RsPrimitiveGLES()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsPrimitiveGLES::create()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsPrimitiveGLES::update()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsPrimitiveGLES::destroy()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// render
void RsPrimitiveGLES::render( eRsPrimitiveType PrimitiveType, BcU32 Offset, BcU32 NoofIndices )
{
	if( pVertexBuffer_ != NULL )
	{
		pVertexBuffer_->bind();
		
		if( pIndexBuffer_ != NULL )
		{
			pIndexBuffer_->bind();
		}

		GLenum GLPrimitiveType = 0;
		switch( PrimitiveType )
		{
			case rsPT_POINTLIST:
				GLPrimitiveType = GL_POINTS;
				break;
			case rsPT_LINELIST:
				GLPrimitiveType = GL_LINES;
				break;
			case rsPT_LINESTRIP:
				GLPrimitiveType = GL_LINE_STRIP;
				break;
			case rsPT_TRIANGLELIST:
				GLPrimitiveType = GL_TRIANGLES;
				break;
			case rsPT_TRIANGLESTRIP:
				GLPrimitiveType = GL_TRIANGLE_STRIP;
				break;
			case rsPT_TRIANGLEFAN:
				GLPrimitiveType = GL_TRIANGLE_FAN;
				break;
		}
		
		if( GLPrimitiveType != 0 )
		{
			glDrawArrays( GLPrimitiveType, Offset, NoofIndices );
		}
	}
}

