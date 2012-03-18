/**************************************************************************
*
* File:		RsPrimitiveGL.cpp
* Author:	Neil Richardson 
* Ver/Date:	4/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/GL/RsPrimitiveGL.h"

#include "System/Renderer/RsCore.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsPrimitiveGL::RsPrimitiveGL( RsVertexBufferGL* pVertexBuffer, RsIndexBufferGL* pIndexBuffer )
{
	pVertexBuffer_ = pVertexBuffer;
	pIndexBuffer_ = pIndexBuffer;
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsPrimitiveGL::~RsPrimitiveGL()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsPrimitiveGL::create()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsPrimitiveGL::update()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsPrimitiveGL::destroy()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// render
void RsPrimitiveGL::render( eRsPrimitiveType PrimitiveType, BcU32 Offset, BcU32 NoofIndices )
{
	if( pVertexBuffer_ != NULL )
	{
		// Bind global state block.
		RsCore::pImpl()->getStateBlock()->bind();

		pVertexBuffer_->bind();
		
		if( pIndexBuffer_ != NULL )
		{
			glEnableClientState( GL_INDEX_ARRAY );
			pIndexBuffer_->bind();
		}
		else
		{
			glDisableClientState( GL_INDEX_ARRAY );
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
			
			// Catch error.
			RsGLCatchError;
		}
	}
}

