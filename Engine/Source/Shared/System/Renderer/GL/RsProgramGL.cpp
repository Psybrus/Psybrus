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
#include "System/Renderer/GL/RsGL.h"

#include "Base/BcString.h"

#include <string>

#include <boost/format.hpp>

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramGL::RsProgramGL( 
	RsContext* pContext, 
	std::vector< RsShader* >&& Shaders, 
	BcU32 NoofVertexAttributes, 
	RsProgramVertexAttribute* pVertexAttributes ):
	RsProgram( pContext ),
	Shaders_( std::move( Shaders ) )
{
	AttributeList_.reserve( NoofVertexAttributes );
	for( BcU32 Idx = 0; Idx < NoofVertexAttributes; ++Idx )
	{
		AttributeList_.push_back( pVertexAttributes[ Idx ] );
	}
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtua
RsProgramGL::~RsProgramGL()
{

}

////////////////////////////////////////////////////////////////////////////////
// create
void RsProgramGL::create()
{
	// Some checks to ensure validity.
	BcAssert( Shaders_.size() > 0 );	

	// Create program.
	GLuint Handle = glCreateProgram();
	BcAssert( Handle != 0 );

	// Attach shaders.
	for( auto* Shader : Shaders_ )
	{
		glAttachShader( Handle, Shader->getHandle< GLuint >() );
		RsGLCatchError();
	}
	
	// Bind all slots up.
	// NOTE: We shouldn't need this in later GL versions with explicit
	//       binding slots.
	for( BcU32 Channel = 0; Channel < 16; ++Channel )
	{
		const std::string Name = boost::str( boost::format( "dcl_Input%1%" ) % Channel );
		glBindAttribLocation( Handle, Channel, Name.c_str() );
		RsGLCatchError();
	}
	
	// Link program.
	glLinkProgram( Handle );
	RsGLCatchError();

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

			RsShaderParameterType InternalType = RsShaderParameterType::INVALID;
			switch( Type )
			{
			case GL_SAMPLER_1D:
				InternalType = RsShaderParameterType::SAMPLER_1D;
				break;
			case GL_SAMPLER_2D:
				InternalType = RsShaderParameterType::SAMPLER_2D;
				break;
			case GL_SAMPLER_3D:
				InternalType = RsShaderParameterType::SAMPLER_3D;
				break;
			case GL_SAMPLER_CUBE:
				InternalType = RsShaderParameterType::SAMPLER_CUBE;
				break;
			case GL_SAMPLER_1D_SHADOW:
				InternalType = RsShaderParameterType::SAMPLER_1D_SHADOW;
				break;
			case GL_SAMPLER_2D_SHADOW:
				InternalType = RsShaderParameterType::SAMPLER_2D_SHADOW;
				break;
			default:
				InternalType = RsShaderParameterType::INVALID;
				break;
			}

			if( InternalType != RsShaderParameterType::INVALID )
			{
				// Add sampler. Will fail if not supported sampler type.
				addSampler( UniformName, UniformLocation, InternalType );
			}
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
		if( UniformBlockNameLength > 0 )
		{
			auto TestIdx = glGetUniformBlockIndex( Handle, UniformBlockName );
			BcAssert( TestIdx == Idx );
			addBlock( UniformBlockName, Idx, Size );
		}
	}

	// Catch error.
	RsGLCatchError();

	// Validate program.
	glValidateProgram( Handle );
	GLint ProgramValidated = 0;
	glGetProgramiv( Handle, GL_VALIDATE_STATUS, &ProgramValidated );
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

	// Bind up sampler slots.
	for( BcU32 Idx = 0; Idx < SamplerList_.size(); ++Idx )
	{
		glProgramUniform1i( Handle, SamplerList_[ Idx ].Handle_, Idx );
		RsGLCatchError();
	}

	// Bind up uniform slots.
	for( BcU32 Idx = 0; Idx < UniformBlockList_.size(); ++Idx )
	{
		glUniformBlockBinding( Handle, UniformBlockList_[ Idx ].Index_, Idx );
		RsGLCatchError();
	}

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
// findSamplerSlot
//virtual
BcU32 RsProgramGL::findSamplerSlot( const BcChar* Name )
{
	BcU32 Idx = 0;
	for( auto It( SamplerList_.begin() ); It != SamplerList_.end(); ++It )
	{
		if( (*It).Name_ == Name )
		{
			return Idx;
		}

		++Idx;
	}

	return BcErrorCode;
}

////////////////////////////////////////////////////////////////////////////////
// findUniformBufferSlot
//virtual
BcU32 RsProgramGL::findUniformBufferSlot( const BcChar* Name )
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
// addParameter
//virtual
const RsProgramVertexAttributeList& RsProgramGL::getVertexAttributeList() const
{
	return AttributeList_;
}

////////////////////////////////////////////////////////////////////////////////
// logShaders
//virtual
void RsProgramGL::logShaders() const
{
	for( auto* Shader : Shaders_ )
	{
		// TODO: Implement.
		//Shader->logShader();
	}
}

////////////////////////////////////////////////////////////////////////////////
// addParameter
void RsProgramGL::addSampler( std::string Name, BcU32 Handle, RsShaderParameterType Type )
{
	// If parameter is valid, add it.
	TSampler Sampler = 
	{
		Name,
		Handle,
		Type,
	};
	SamplerList_.push_back( Sampler );
}

////////////////////////////////////////////////////////////////////////////////
// addBlock
void RsProgramGL::addBlock( std::string Name, BcU32 Handle, BcU32 Size )
{
	TUniformBlock Block = 
	{
		Name,
		Handle,
		Size
	};

	UniformBlockList_.push_back( Block );
}
