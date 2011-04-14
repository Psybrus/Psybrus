/**************************************************************************
*
* File:		RsProgramGLES.h
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSPROGRAMGLES_H__
#define __RSPROGRAMGLES_H__

#include "RsGPUResourceGLES.h"
#include "RsShaderGLES.h"
#include "RsTextureGLES.h"
#include "RsProgram.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsProgramParameterGLES;
class RsProgramGLES;

////////////////////////////////////////////////////////////////////////////////
// RsProgramParameterGLES
class RsProgramParameterGLES:
	public RsProgramParameter
{
public:
	RsProgramParameterGLES( const std::string& Name, RsProgramGLES* pParent, GLuint Parameter, GLuint Sampler );
	virtual ~RsProgramParameterGLES();
	
	void								setInt( BcS32 Value );
	void								setFloat( BcReal Value );
	void								setVector( const BcVec2d& Value );
	void								setVector( const BcVec3d& Value );
	void								setVector( const BcVec4d& Value );
	void								setMatrix( const BcMat4d& Value );
	void								setTexture( RsTexture* pTexture );

	/**
	 * Bind the parameter that has been set if need be.
	 */
	void								bind();
	
private:
	RsProgramGLES*						pParent_;
	GLint								Parameter_;
	
	// Sampler information. NOTE: Subclass this to put more texture info in.
	GLuint								Sampler_;
	RsTextureGLES*						pTexture_;
};

////////////////////////////////////////////////////////////////////////////////
// RsProgramGLES
class RsProgramGLES:
	public RsProgram,
	public RsGPUResourceGLES
{
public:
	RsProgramGLES( RsShaderGLES* pVertexShader, RsShaderGLES* pFragmentShader );
	virtual ~RsProgramGLES();
	
	void								create();
	void								update();
	void								destroy();	
	
	virtual RsProgramParameter*			findParameter( const std::string& Name, BcBool IsSampler );
	virtual void						bind();

private:	
	void								bindAttribute( eRsVertexChannel Channel, const BcChar* Name );

private:
	typedef std::list< RsProgramParameterGLES* >	TParameterList;
	typedef TParameterList::iterator				TParameterListIterator;
	TParameterList						ParameterList_;

	RsShaderGLES*						pVertexShader_;
	RsShaderGLES*						pFragmentShader_;	
	
	BcU32								TotalSampler_;
};

#endif
