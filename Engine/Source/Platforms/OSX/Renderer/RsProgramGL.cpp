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

#include "RsProgramGL.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramParameterGL::RsProgramParameterGL( const std::string& Name, RsProgramGL* pParent, GLuint Parameter, GLuint Sampler ):
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
RsProgramParameterGL::~RsProgramParameterGL()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// setInt
void RsProgramParameterGL::setInt( BcS32 Value )
{
	glUniform1i( Parameter_, Value );
	RsGLCatchError;
}

////////////////////////////////////////////////////////////////////////////////
// setFloat
void RsProgramParameterGL::setFloat( BcReal Value )
{
	glUniform1f( Parameter_, Value );
	RsGLCatchError;
}

////////////////////////////////////////////////////////////////////////////////
// setVector
void RsProgramParameterGL::setVector( const BcVec2d& Value )
{
	glUniform2fv( Parameter_, 1, (GLfloat*)&Value );
	RsGLCatchError;
}

////////////////////////////////////////////////////////////////////////////////
// setVector
void RsProgramParameterGL::setVector( const BcVec3d& Value )
{
	glUniform3fv( Parameter_, 1, (GLfloat*)&Value );
	RsGLCatchError;
}

////////////////////////////////////////////////////////////////////////////////
// setVector
void RsProgramParameterGL::setVector( const BcVec4d& Value )
{
	glUniform4fv( Parameter_, 1, (GLfloat*)&Value );
	RsGLCatchError;
}

////////////////////////////////////////////////////////////////////////////////
// setMatrix
void RsProgramParameterGL::setMatrix( const BcMat4d& Value )
{
	glUniformMatrix4fv( Parameter_, 1, GL_FALSE, (GLfloat*)&Value );
	RsGLCatchError;
}

////////////////////////////////////////////////////////////////////////////////
// update
void RsProgramParameterGL::setTexture( RsTexture* pTexture )
{
	if( Sampler_ != -1 )
	{
		setInt( Sampler_ );
		pTexture_ = static_cast< RsTextureGL* >( pTexture );
	}
}

////////////////////////////////////////////////////////////////////////////////
// setMatrix
void RsProgramParameterGL::bind()
{
	if( Sampler_ != -1 )
	{
		glActiveTexture( GL_TEXTURE0 + Sampler_ );
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, pTexture_ ? pTexture_->getHandle< GLuint >() : 0 );
	
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );	

		RsGLCatchError;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramGL::RsProgramGL( RsShaderGL* pVertexShader, RsShaderGL* pFragmentShader ):
	pVertexShader_( pVertexShader ),
	pFragmentShader_( pFragmentShader )
{
	
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtua
RsProgramGL::~RsProgramGL()
{
	// Delete all parameters that we've cached.
	for( TParameterListIterator It( ParameterList_.begin() ); It != ParameterList_.end(); ++It )
	{
		delete (*It);
	}
}

////////////////////////////////////////////////////////////////////////////////
// create
void RsProgramGL::create()
{
	// Some checks to ensure validity.
	BcAssert( pVertexShader_ != NULL );
	BcAssert( pFragmentShader_ != NULL );
	BcAssert( pVertexShader_->getHandle< GLuint >() != 0 );
	BcAssert( pFragmentShader_->getHandle< GLuint >() != 0 );	

	// Create program.
	GLuint Handle = glCreateProgram();
	BcAssert( Handle != 0 );

	// Set handle.
	setHandle( Handle );

	// Attach shaders.
	glAttachShader( Handle, pVertexShader_->getHandle< GLuint >() );
	glAttachShader( Handle, pFragmentShader_->getHandle< GLuint >() );
	
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
	glLinkProgram( Handle );	


	// Catch error.
	RsGLCatchError;
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
RsProgramParameter* RsProgramGL::findParameter( const std::string& Name, BcBool IsSampler )
{
	// NOTE: Lazy finding. Could store these at build/load time.
	RsProgramParameterGL* pParameter = NULL;

	GLuint Handle = getHandle< GLuint >();

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
		GLint UniformLocation = glGetUniformLocation( Handle, Name.c_str() );
		
		if( UniformLocation != GLint( -1 ) )
		{
			GLuint Sampler = IsSampler ? TotalSampler_++ : -1;
			pParameter = new RsProgramParameterGL( Name, this, UniformLocation, Sampler );
			ParameterList_.push_back( pParameter );
		}
		
	}
	
	return pParameter;
}

////////////////////////////////////////////////////////////////////////////////
// bind
//virtual
void RsProgramGL::bind()
{
	GLuint Handle = getHandle< GLuint >();

	glUseProgram( Handle );
	RsGLCatchError;
	
	// Bind all parameters.
	for( TParameterListIterator It( ParameterList_.begin() ); It != ParameterList_.end(); ++It )
	{
		RsProgramParameterGL* pParameter = (*It);
		pParameter->bind();
	}
}

////////////////////////////////////////////////////////////////////////////////
// bindAttribute
void RsProgramGL::bindAttribute( eRsVertexChannel Channel, const BcChar* Name )
{
	GLuint Handle = getHandle< GLuint >();

	glBindAttribLocation( Handle, Channel, Name );
	if( glGetError() != GL_NO_ERROR )
	{
		BcPrintf( "WARNING: RsProgramGL: Could not bind attribute \"%s\"\n", Name );
	}
	
}

