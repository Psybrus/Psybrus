/**************************************************************************
*
* File:		RsProgramGL.h
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSPROGRAMGL_H__
#define __RSPROGRAMGL_H__

#include "RsShaderGL.h"
#include "RsTextureGL.h"
#include "RsProgram.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsProgramGL;

////////////////////////////////////////////////////////////////////////////////
// RsProgramGL
class RsProgramGL:
	public RsProgram
{
public:
	RsProgramGL( RsShaderGL* pVertexShader, RsShaderGL* pFragmentShader );
	virtual ~RsProgramGL();
	
	void								create();
	void								update();
	void								destroy();	
	
	virtual BcU32						getParameterBufferSize() const;
	virtual BcU32						findParameterOffset( const BcName& Name, eRsShaderParameterType& Type, BcU32& Offset ) const;
	virtual void						bind( void* pParameterBuffer );

private:	
	void								bindAttribute( eRsVertexChannel Channel, const BcChar* Name );
	void								addParameter( const GLchar* pName, GLint Handle, GLenum Type );
	
private:
	struct TParameter
	{
		BcName							Name_;
		GLint							Handle_;
		BcU32							Offset_;
		eRsShaderParameterType			Type_;
	};
	
	typedef std::list< TParameter > TParameterList;
	typedef TParameterList::iterator TParameterListIterator;
	typedef TParameterList::const_iterator TParameterListConstIterator;
	TParameterList						ParameterList_;
	BcU32								ParameterBufferSize_;

	RsShaderGL*							pVertexShader_;
	RsShaderGL*							pFragmentShader_;
};

#endif
