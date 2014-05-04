/**************************************************************************
*
* File:		RsProgramGL.cpp
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/GL/RsProgramGL.h"

#include "Base/BcString.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramGL::RsProgramGL( RsContext* pContext, BcU32 NoofShaders, RsShader** ppShaders, BcU32 NoofVertexAttributes, RsProgramVertexAttribute* pVertexAttributes ):
	RsProgram( pContext ),
	ParameterBufferSize_( 0 )
{
	NoofShaders_ = NoofShaders;
	ppShaders_ = new RsShaderGL*[ NoofShaders ];	

	for( BcU32 Idx = 0; Idx < NoofShaders_; ++Idx )
	{
		ppShaders_[ Idx ] = static_cast< RsShaderGL* >( ppShaders[ Idx ] );
	}

	AttributeList_.reserve( NoofVertexAttributes );
	for( BcU32 Idx = 0; Idx < NoofVertexAttributes; ++Idx )
	{
		TAttribute Attribute = { *pVertexAttributes[ Idx ].AttributeName_, pVertexAttributes[ Idx ].Channel_ };
		AttributeList_.push_back( Attribute );
	}
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtua
RsProgramGL::~RsProgramGL()
{
	delete [] ppShaders_;
	ppShaders_ = NULL;
	NoofShaders_ = 0;
}

////////////////////////////////////////////////////////////////////////////////
// create
void RsProgramGL::create()
{
	// Some checks to ensure validity.
	BcAssert( ppShaders_ != NULL );
	BcAssert( NoofShaders_ > 0 );	

	// Create program.
	GLuint Handle = glCreateProgram();
	BcAssert( Handle != 0 );

	// Attach shaders.
	for( BcU32 Idx = 0; Idx < NoofShaders_; ++Idx )
	{
		glAttachShader( Handle, ppShaders_[ Idx ]->getHandle< GLuint >() );

		//ppShaders_[ Idx ]->logShader();
	}
	
	// Bind default vertex attributes.
	for( auto& Attribute : AttributeList_ )
	{
		bindAttribute( Handle, Attribute.Channel_, Attribute.Name_.c_str() );
	}
	
	// Link program.
	glLinkProgram( Handle );
	RsGLCatchError;

	GLint ProgramLinked = 0;
	glGetProgramiv( Handle, GL_LINK_STATUS, &ProgramLinked );
	if ( !ProgramLinked )
	{					 
		// There was an error here, first get the length of the log message.
		int i32InfoLogLength, i32CharsWritten; 
		glGetProgramiv( Handle, GL_INFO_LOG_LENGTH, &i32InfoLogLength );

		// Allocate enough space for the message, and retrieve it.
		char* pszInfoLog = new char[i32InfoLogLength];
		glGetProgramInfoLog( Handle, i32InfoLogLength, &i32CharsWritten, pszInfoLog );
		BcPrintf( "RsProgramGL: Infolog:\n %s\n", pszInfoLog );
		delete [] pszInfoLog;

		destroy();
		return;
	}
	
	// Clear parameter list and buffer.
	ParameterList_.clear();
	ParameterBufferSize_ = 0;

	// Attempt to find uniform names.
	GLint ActiveUniforms = 0;
	glGetProgramiv( Handle, GL_ACTIVE_UNIFORMS, &ActiveUniforms );
	
	for( BcU32 Idx = 0; Idx < (BcU32)ActiveUniforms; ++Idx )
	{
		// Uniform information.
		GLchar UniformName[ 256 ];
		GLsizei UniformNameLength = 0;
		GLint Size = 0;
		GLenum Type = GL_INVALID_VALUE;

		// Get the uniform.
		glGetActiveUniform( Handle, Idx, sizeof( UniformName ), &UniformNameLength, &Size, &Type, UniformName );
		
		// Add it as a parameter.
		if( UniformNameLength > 0 && Type != GL_INVALID_VALUE )
		{
			GLint UniformLocation = glGetUniformLocation( Handle, UniformName );

			// Trim index off.
			BcChar* pIndexStart = BcStrStr( UniformName, "[0]" );
			if( pIndexStart != NULL )
			{
				*pIndexStart = '\0';
			}

			addParameter( UniformName, UniformLocation, Type, Size );
		}
	}
	
	// Attempt to find uniform block names.
	GLint ActiveUniformBlocks = 0;
	glGetProgramiv( Handle, GL_ACTIVE_UNIFORM_BLOCKS, &ActiveUniformBlocks );
	
	for( BcU32 Idx = 0; Idx < (BcU32)ActiveUniformBlocks; ++Idx )
	{
		// Uniform information.
		GLchar UniformBlockName[ 256 ];
		GLsizei UniformBlockNameLength = 0;
		GLint Size = 0;

		// Get the uniform block size.
		glGetActiveUniformBlockiv( Handle, Idx, GL_UNIFORM_BLOCK_DATA_SIZE, &Size );
		glGetActiveUniformBlockName( Handle, Idx, sizeof( UniformBlockName ), &UniformBlockNameLength, UniformBlockName );
		
		// Add it as a parameter.
		if( UniformBlockNameLength > 0  )
		{
			auto TestIdx = glGetUniformBlockIndex( Handle, UniformBlockName );
			BcAssert( TestIdx == Idx );
			addBlock( UniformBlockName, Idx, Size );
		}
	}

	// Catch error.
	RsGLCatchError;

	// Bind/unbind to ensure it works.
	glUseProgram( Handle );
	RsGLCatchError;
	glUseProgram( 0 );

	// Set handle.
	setHandle( Handle );
}

////////////////////////////////////////////////////////////////////////////////
// update
void RsProgramGL::update()
{

}

////////////////////////////////////////////////////////////////////////////////
// destroy
void RsProgramGL::destroy()
{
	GLuint Handle = getHandle< GLuint >();
	if( Handle != 0 )
	{
		glDeleteProgram( Handle );
		setHandle< GLuint >( 0 );
	}
}

////////////////////////////////////////////////////////////////////////////////
// findParameter
//virtual
BcU32 RsProgramGL::getParameterBufferSize() const
{
	return ParameterBufferSize_;
}

////////////////////////////////////////////////////////////////////////////////
// findParameterOffset
//virtual
BcU32 RsProgramGL::findParameterOffset( const BcChar* Name, eRsShaderParameterType& Type, BcU32& Offset, BcU32& Bytes ) const
{
	for( TParameterListConstIterator It( ParameterList_.begin() ); It != ParameterList_.end(); ++It )
	{
		if( (*It).Name_ == Name )
		{
			Type = (*It).Type_;
			Offset = (*It).Offset_;
			Bytes = (*It).TypeBytes_;
			return BcTrue;
		}
	}
	
	return BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// bind
//virtual
void RsProgramGL::bind( void* pParameterBuffer )
{
	GLuint Handle = getHandle< GLuint >();
	glUseProgram( Handle );
	RsGLCatchError;
	
	// Bind parameters from buffer if we have been given one.
	// TODO: Only set a value if it isn't equal to the last set one.
	if( pParameterBuffer != NULL )
	{
		BcF32* pFloatParameter = (BcF32*)pParameterBuffer;
		BcS32* pIntParameter = (BcS32*)pParameterBuffer;
		for( TParameterListIterator It( ParameterList_.begin() ); It != ParameterList_.end(); ++It )
		{
			TParameter& Parameter = (*It);
			const GLint ParamHandle = Parameter.Handle_;
			const BcU32 Offset = Parameter.Offset_;
			const BcU32 Count = Parameter.Size_;
#ifdef PSY_DEBUG
			BcF32* pFloatParameterOffset = &pFloatParameter[ Offset ];
			BcS32* pIntParameterOffset = &pIntParameter[ Offset ];
#endif
			
			switch( Parameter.Type_ )
			{
				case rsSPT_FLOAT:
					glUniform1fv( ParamHandle, Count, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_FLOAT_VEC2:
					glUniform2fv( ParamHandle, Count, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_FLOAT_VEC3:
					glUniform3fv( ParamHandle, Count, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_FLOAT_VEC4:
					glUniform4fv( ParamHandle, Count, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_FLOAT_MAT2:
					glUniformMatrix2fv( ParamHandle, Count, GL_FALSE, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_FLOAT_MAT3:
					glUniformMatrix3fv( ParamHandle, Count, GL_FALSE, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_FLOAT_MAT4:
					glUniformMatrix4fv( ParamHandle, Count, GL_FALSE, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_INT:
				case rsSPT_BOOL:
				case rsSPT_SAMPLER_1D:
				case rsSPT_SAMPLER_2D:
				case rsSPT_SAMPLER_3D:
				case rsSPT_SAMPLER_CUBE:
				case rsSPT_SAMPLER_1D_SHADOW:
				case rsSPT_SAMPLER_2D_SHADOW:
					glUniform1iv( ParamHandle, Count, &pIntParameter[ Offset ] );
					break;
				case rsSPT_INT_VEC2:
				case rsSPT_BOOL_VEC2:
					glUniform2iv( ParamHandle, Count, &pIntParameter[ Offset ] );
					break;
				case rsSPT_INT_VEC3:
				case rsSPT_BOOL_VEC3:
					glUniform3iv( ParamHandle, Count, &pIntParameter[ Offset ] );
					break;
				case rsSPT_INT_VEC4:
				case rsSPT_BOOL_VEC4:
					glUniform4iv( ParamHandle, Count, &pIntParameter[ Offset ] );
					break;
					break;
			}
			
#ifdef PSY_DEBUG
			int Error = glGetError();
			if( Error != 0 )
			{
				BcPrintf( "Error setting parameter \"%s\". Handle=%u, f=%f, i=%u\n", Parameter.Name_.c_str(), ParamHandle, *pFloatParameterOffset, *pIntParameterOffset );
			}
#endif
		}
	}

	// Bind up uniform blocks.
	BcU32 BindingPoint = 0;
	for( auto It( UniformBlockList_.begin() ); It != UniformBlockList_.end(); ++It )
	{
		if( (*It).Buffer_ != nullptr )
		{
			glUniformBlockBinding( Handle, (*It).Index_, BindingPoint );
			glBindBufferRange( GL_UNIFORM_BUFFER, BindingPoint, (*It).Buffer_->getHandle< GLuint >(), 0, (*It).Buffer_->getDataSize() );

			++BindingPoint;
			RsGLCatchError;
		}
	}
		
}

////////////////////////////////////////////////////////////////////////////////
// findUniformBlockIndex
//virtual
BcU32 RsProgramGL::findUniformBlockIndex( const BcChar* Name )
{
	for( auto It( UniformBlockList_.begin() ); It != UniformBlockList_.end(); ++It )
	{
		if( (*It).Name_ == Name )
		{
			return (*It).Index_;
		}
	}

	return BcErrorCode;
}

////////////////////////////////////////////////////////////////////////////////
// setUniformBlock
//virtual
void RsProgramGL::setUniformBlock( BcU32 Index, RsUniformBuffer* Buffer )
{
	auto& UniformBlock( UniformBlockList_[ Index ] );
	if (Buffer != nullptr)
	{
		BcAssert( Buffer->getDataSize() == UniformBlock.Size_ );
	}
	UniformBlock.Buffer_ = Buffer;
}

////////////////////////////////////////////////////////////////////////////////
// bindAttribute
void RsProgramGL::bindAttribute( GLuint ProgramHandle, eRsVertexChannel Channel, const BcChar* Name )
{
	glBindAttribLocation( ProgramHandle, Channel, Name );
	if( glGetError() != GL_NO_ERROR )
	{
		BcPrintf( "WARNING: RsProgramGL: Could not bind attribute \"%s\"\n", Name );
	}	
}

////////////////////////////////////////////////////////////////////////////////
// addParameter
void RsProgramGL::addParameter( const GLchar* pName, GLint Handle, GLenum Type, BcU32 Size )
{
	// Calculate number of bytes it needs and size.
	eRsShaderParameterType InternalType;
	BcU32 Bytes = 0;
	switch( Type )
	{
		case GL_FLOAT:
			InternalType = rsSPT_FLOAT;
			Bytes = 4;
			break;
		case GL_FLOAT_VEC2:
			InternalType = rsSPT_FLOAT_VEC2;
			Bytes = 8;
			break;
		case GL_FLOAT_VEC3:
			InternalType = rsSPT_FLOAT_VEC3;
			Bytes = 12;
			break;
		case GL_FLOAT_VEC4:
			InternalType = rsSPT_FLOAT_VEC4;
			Bytes = 16;
			break;
		case GL_INT:
			InternalType = rsSPT_INT;
			Bytes = 4;
			break;
		case GL_INT_VEC2:
			InternalType = rsSPT_INT_VEC2;
			Bytes = 8;
			break;
		case GL_INT_VEC3:
			InternalType = rsSPT_INT_VEC3;
			Bytes = 12;
			break;
		case GL_INT_VEC4:
			InternalType = rsSPT_INT_VEC4;
			Bytes = 16;
			break;
		case GL_BOOL:
			InternalType = rsSPT_BOOL;
			Bytes = 4;
			break;
		case GL_BOOL_VEC2:
			InternalType = rsSPT_BOOL_VEC2;
			Bytes = 8;
			break;
		case GL_BOOL_VEC3:
			InternalType = rsSPT_BOOL_VEC3;
			Bytes = 12;
			break;
		case GL_BOOL_VEC4:
			InternalType = rsSPT_BOOL_VEC4;
			Bytes = 16;
			break;
		case GL_FLOAT_MAT2:
			InternalType = rsSPT_FLOAT_MAT2;
			Bytes = 16;
			break;
		case GL_FLOAT_MAT3:
			InternalType = rsSPT_FLOAT_MAT3;
			Bytes = 36;
			break;
		case GL_FLOAT_MAT4:
			InternalType = rsSPT_FLOAT_MAT4;
			Bytes = 64;
			break;
			/* NOTE: GL2.1 or later, ignore for now!
		case GL_FLOAT_MAT2x3:
			Bytes = 24;
			break;
		case GL_FLOAT_MAT2x4:
			Bytes = 32;
			break;
		case GL_FLOAT_MAT3x2:
			Bytes = 24;
			break;
		case GL_FLOAT_MAT3x4:
			Bytes = 48;
			break;
		case GL_FLOAT_MAT4x2:
			Bytes = 32;
			break;
		case GL_FLOAT_MAT4x3:
			Bytes = 48;
			break;
			*/
		case GL_SAMPLER_1D:
			InternalType = rsSPT_SAMPLER_1D;
			Bytes = 4;
			break;
		case GL_SAMPLER_2D:
			InternalType = rsSPT_SAMPLER_2D;
			Bytes = 4;
			break;
		case GL_SAMPLER_3D:
			InternalType = rsSPT_SAMPLER_3D;
			Bytes = 4;
			break;
		case GL_SAMPLER_CUBE:
			InternalType = rsSPT_SAMPLER_CUBE;
			Bytes = 4;
			break;
		case GL_SAMPLER_1D_SHADOW:
			InternalType = rsSPT_SAMPLER_1D_SHADOW;
			Bytes = 4;
			break;
		case GL_SAMPLER_2D_SHADOW:
			InternalType = rsSPT_SAMPLER_2D_SHADOW;
			Bytes = 4;
			break;
		default:
			InternalType = rsSPT_INVALID;
			Bytes = 0;
			break;
	}

	// If parameter is valid, add it.
	if( InternalType != rsSPT_INVALID )
	{
		TParameter Parameter = 
		{
			pName,
			Handle,
			ParameterBufferSize_ >> 2,
			Size,
			Bytes,
			InternalType
		};
		
		// Add parameter.
		ParameterList_.push_back( Parameter );
	
		// Increate parameter buffer size.
		ParameterBufferSize_ += Bytes * Size;
		
		// Log.
		//BcPrintf( "RsProgramGL::Adding parameter \"%s\". Handle=%u, Offset=%u\n", pName, Handle, ParameterBufferSize_ - Bytes );
	}
}

////////////////////////////////////////////////////////////////////////////////
// addBlock
void RsProgramGL::addBlock( const GLchar* pName, GLint Handle, BcU32 Size )
{
	TUniformBlock Block = 
	{
		pName,
		Handle,
		Size
	};

	UniformBlockList_.push_back( Block );
}
