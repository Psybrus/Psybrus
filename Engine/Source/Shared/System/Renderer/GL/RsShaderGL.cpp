/**************************************************************************
*
* File:		RsShaderGL.cpp
* Author:	Neil Richardson 
* Ver/Date:	25/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/GL/RsShaderGL.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsShaderGL::RsShaderGL( eRsShaderType ShaderType, eRsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize )
{
	// Setup basic parameters.
	switch( ShaderType )
	{
		case rsST_VERTEX:
			Type_ = GL_VERTEX_SHADER;
			break;

		case rsST_TESSELATION_CONTROL:
			Type_ = GL_TESS_CONTROL_SHADER;
			break;

		case rsST_TESSELATION_EVALUATION:
			Type_ = GL_TESS_EVALUATION_SHADER;
			break;

		case rsST_GEOMETRY:
			Type_ = GL_GEOMETRY_SHADER;
			break;

		case rsST_FRAGMENT:
			Type_ = GL_FRAGMENT_SHADER;
			break;
			
		case rsST_COMPUTE:
			Type_ = GL_COMPUTE_SHADER;
			break;

		default:
			BcBreakpoint;
			break;
	}
	DataType_ = ShaderDataType;
	pData_ = pShaderData;
	DataSize_ = ShaderDataSize;
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsShaderGL::~RsShaderGL()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsShaderGL::create()
{
	// Create handle for shader.
	GLuint Handle = glCreateShader( Type_ );
	setHandle( Handle );
	RsGLCatchError;
	
	if( Handle != 0 )
	{
		switch( DataType_ )
		{
			case rsSDT_BINARY:
			{
				// Shader data can't be updated later, so set it all up now.
				//glShaderBinary( 1, &Handle_, Format_, pData_, (GLint)DataSize_ );
				BcBreakpoint; // WAS A GLES THING.
			}
			break;
				
			case rsSDT_SOURCE:
			{
				// NOTE: Oolong stuff.
				
				// Load the source code into it.
				glShaderSource( Handle, 1, (const GLchar**)&pData_, NULL );
				RsGLCatchError;
				
				// Compile the source code.
				glCompileShader( Handle );
				RsGLCatchError;
				
				// Test if compilation succeeded.
				GLint ShaderCompiled;
				glGetShaderiv( Handle, GL_COMPILE_STATUS, &ShaderCompiled );
				if ( !ShaderCompiled )
				{					 
					// There was an error here, first get the length of the log message.
					int i32InfoLogLength, i32CharsWritten; 
					glGetShaderiv( Handle, GL_INFO_LOG_LENGTH, &i32InfoLogLength );
					
					// Allocate enough space for the message, and retrieve it.
					char* pszInfoLog = new char[i32InfoLogLength];
					glGetShaderInfoLog( Handle, i32InfoLogLength, &i32CharsWritten, pszInfoLog );

					BcPrintf( "=======================================================\n" );
					BcPrintf( "Error Compiling shader:\n" );
					BcPrintf( "=======================================================\n" );
					logShader();
					BcPrintf( "=======================================================\n" );
					BcPrintf( "RsShaderGL: Infolog:\n %s\n", pszInfoLog );
					BcPrintf( "=======================================================\n" );
					delete [] pszInfoLog;

					destroy();
				}
				
			}
			break;
		}
	}
	
	// Destroy if there is a failure.
	GLenum Error = glGetError();
	if ( Error != GL_NO_ERROR )
	{
		BcPrintf( "RsShaderGL: Error has occured: %u\n", Error );
		destroy();
	}
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsShaderGL::update()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsShaderGL::destroy()
{
	GLuint Handle = getHandle< GLuint >();
	if( Handle != 0 )
	{
		glDeleteShader( Handle );
		setHandle< GLuint >( Handle );
	}
}

////////////////////////////////////////////////////////////////////////////////
// logShader
void RsShaderGL::logShader()
{
	BcPrintf( "%s\n", pData_ );
}
