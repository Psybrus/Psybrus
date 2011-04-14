/**************************************************************************
*
* File:		RsProgramGLES.cpp
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsProgramGLES.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramParameterGLES::RsProgramParameterGLES( const std::string& Name, RsProgramGLES* pParent, GLuint Parameter, GLuint Sampler ):
	RsProgramParameter( Name ),
	pParent_( pParent ),
	Parameter_( Parameter ),
	Sampler_( Sampler ),
	pTexture_( NULL )
{
	
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsProgramParameterGLES::~RsProgramParameterGLES()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// setInt
void RsProgramParameterGLES::setInt( BcS32 Value )
{
	glUniform1i( Parameter_, Value );
}

////////////////////////////////////////////////////////////////////////////////
// setFloat
void RsProgramParameterGLES::setFloat( BcReal Value )
{
	glUniform1f( Parameter_, Value );
}

////////////////////////////////////////////////////////////////////////////////
// setVector
void RsProgramParameterGLES::setVector( const BcVec2d& Value )
{
	glUniform2fv( Parameter_, 1, (GLfloat*)&Value );
}

////////////////////////////////////////////////////////////////////////////////
// setVector
void RsProgramParameterGLES::setVector( const BcVec3d& Value )
{
	glUniform3fv( Parameter_, 1, (GLfloat*)&Value );
}

////////////////////////////////////////////////////////////////////////////////
// setVector
void RsProgramParameterGLES::setVector( const BcVec4d& Value )
{
	glUniform4fv( Parameter_, 1, (GLfloat*)&Value );
}

////////////////////////////////////////////////////////////////////////////////
// setMatrix
void RsProgramParameterGLES::setMatrix( const BcMat4d& Value )
{
	glUniformMatrix4fv( Parameter_, 1, GL_FALSE, (GLfloat*)&Value );
}

////////////////////////////////////////////////////////////////////////////////
// update
void RsProgramParameterGLES::setTexture( RsTexture* pTexture )
{
	if( Sampler_ != -1 )
	{
		setInt( Sampler_ );
		pTexture_ = static_cast< RsTextureGLES* >( pTexture );
	}
}

////////////////////////////////////////////////////////////////////////////////
// setMatrix
void RsProgramParameterGLES::bind()
{
	if( Sampler_ != -1 )
	{
		glActiveTexture( GL_TEXTURE0 + Sampler_ );
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, pTexture_ ? pTexture_->getHandle() : 0 );
	
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );	
	}
}

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramGLES::RsProgramGLES( RsShaderGLES* pVertexShader, RsShaderGLES* pFragmentShader ):
	pVertexShader_( pVertexShader ),
	pFragmentShader_( pFragmentShader )
{
	
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtua
RsProgramGLES::~RsProgramGLES()
{
	// Delete all parameters that we've cached.
	for( TParameterListIterator It( ParameterList_.begin() ); It != ParameterList_.end(); ++It )
	{
		delete (*It);
	}
}

////////////////////////////////////////////////////////////////////////////////
// create
void RsProgramGLES::create()
{
	// Some checks to ensure validity.
	BcAssert( pVertexShader_ != NULL );
	BcAssert( pFragmentShader_ != NULL );
	BcAssert( pVertexShader_->getHandle() != 0 );
	BcAssert( pFragmentShader_->getHandle() != 0 );	

	// Create program.
	Handle_ = glCreateProgram();
	BcAssert( Handle_ != 0 );

	// Attach shaders.
	glAttachShader( Handle_, pVertexShader_->getHandle() );
	glAttachShader( Handle_, pFragmentShader_->getHandle() );
	
	// Bind attributes.
	bindAttribute( rsVC_POSITION,		"aPosition" );
	bindAttribute( rsVC_NORMAL,			"aNormal" );
	bindAttribute( rsVC_TANGENT,		"aTangent" );
	bindAttribute( rsVC_TEXCOORD0,		"aTexCoord0" );
	bindAttribute( rsVC_TEXCOORD1,		"aTexCoord1" );
	bindAttribute( rsVC_TEXCOORD2,		"aTexCoord2" );
	bindAttribute( rsVC_TEXCOORD3,		"aTexCoord3" );
	bindAttribute( rsVC_COLOUR,			"aColour" );
	
	// Link program.
	glLinkProgram( Handle_ );	

	// Clear GL error.
	
}

////////////////////////////////////////////////////////////////////////////////
// update
void RsProgramGLES::update()
{

}

////////////////////////////////////////////////////////////////////////////////
// destroy
void RsProgramGLES::destroy()
{
	if( Handle_ != 0 )
	{
		glDeleteProgram( Handle_ );
		Handle_ = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// findParameter
//virtual
RsProgramParameter* RsProgramGLES::findParameter( const std::string& Name, BcBool IsSampler )
{
	// NOTE: Lazy finding. Could store these at build/load time.
	RsProgramParameterGLES* pParameter = NULL;
	
	// Try to find in list first.
	for( TParameterListIterator It( ParameterList_.begin() ); It != ParameterList_.end(); ++It )
	{
		if( (*It)->getName() == Name )
		{
			pParameter = (*It);
			break;
		}
	}
	
	// Lookup and add to list if it NULL (if we can find it).
	if( pParameter == NULL )
	{
		GLint UniformLocation = glGetUniformLocation( Handle_, Name.c_str() );
		
		if( UniformLocation != GLint( -1 ) )
		{
			GLuint Sampler = IsSampler ? TotalSampler_++ : -1;
			pParameter = new RsProgramParameterGLES( Name, this, UniformLocation, Sampler );
			ParameterList_.push_back( pParameter );
		}
		
	}
	
	return pParameter;
}

////////////////////////////////////////////////////////////////////////////////
// bind
//virtual
void RsProgramGLES::bind()
{
	glUseProgram( Handle_ );
	
	// Bind all parameters.
	for( TParameterListIterator It( ParameterList_.begin() ); It != ParameterList_.end(); ++It )
	{
		RsProgramParameterGLES* pParameter = (*It);
		pParameter->bind();
	}
}

////////////////////////////////////////////////////////////////////////////////
// bindAttribute
void RsProgramGLES::bindAttribute( eRsVertexChannel Channel, const BcChar* Name )
{
	glBindAttribLocation( Handle_, Channel, Name );
	if( glGetError() != GL_NO_ERROR )
	{
		BcPrintf( "WARNING: RsProgramGLES: Could not bind attribute \"%s\"\n", Name );
	}
	
}

