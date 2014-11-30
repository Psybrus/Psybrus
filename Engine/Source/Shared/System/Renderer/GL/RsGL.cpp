/**************************************************************************
*
* File:		RsGL.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		GL includes.
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/GL/RsGL.h"

////////////////////////////////////////////////////////////////////////////////
// RsGLCatchError
void RsGLCatchError()
{
	BcU32 TotalErrors = 0;
	GLuint Error;
	do
	{
		Error = glGetError();
#if PSY_DEBUG
		std::string ErrorString = "UNKNOWN";
		switch( Error )
		{
		case GL_INVALID_ENUM:
			ErrorString = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			ErrorString = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			ErrorString = "GL_INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			ErrorString = "GL_STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			ErrorString = "GL_STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			ErrorString = "GL_OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			ErrorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_TABLE_TOO_LARGE:
			ErrorString = "GL_TABLE_TOO_LARGE";
			break;
		}

		if( Error != 0 )
		{
			BcPrintf( "RsGL: Error: %s\n", ErrorString.c_str() );
			++TotalErrors;
		}
#endif
	}
	while( Error != 0 );

	if( TotalErrors > 0 )
	{
		BcBreakpoint;
	}
}
