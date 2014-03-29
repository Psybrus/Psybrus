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

#include "System/Renderer/GL/RsShaderGL.h"
#include "System/Renderer/GL/RsUniformBufferGL.h"
#include "System/Renderer/RsProgram.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsProgramGL;

////////////////////////////////////////////////////////////////////////////////
// RsProgramGL
class RsProgramGL:
	public RsProgram
{
public:
	RsProgramGL( RsContext* pContext, BcU32 NoofShaders, RsShader** ppShaders );
	virtual ~RsProgramGL();
	
	void								create();
	void								update();
	void								destroy();	
	
	// Old, dirty interface.
	virtual BcU32						getParameterBufferSize() const;
	virtual BcU32						findParameterOffset( const BcChar* Name, eRsShaderParameterType& Type, BcU32& Offset, BcU32& TypeBytes ) const;
	virtual void						bind( void* pParameterBuffer );

	// New, shiny interface!
	virtual BcU32						findUniformBlockIndex( const BcChar* Name );
	virtual void						setUniformBlock( BcU32 Index, RsUniformBuffer* Buffer );

private:	
	void								bindAttribute( GLuint ProgramHandle, eRsVertexChannel Channel, const BcChar* Name );
	void								addParameter( const GLchar* pName, GLint Handle, GLenum Type, BcU32 Size );
	void								addBlock( const GLchar* pName, GLint Handle, BcU32 Size );
	
private:
	struct TParameter
	{
		std::string						Name_;
		GLint							Handle_;
		BcU32							Offset_;
		BcU32							Size_;
		BcU32							TypeBytes_;
		eRsShaderParameterType			Type_;
	};

	struct TUniformBlock
	{
		std::string						Name_;
		GLint							Index_;
		BcU32							Size_;
		RsUniformBuffer*				Buffer_;
	};

	typedef std::vector< TParameter > TParameterList;
	typedef TParameterList::iterator TParameterListIterator;
	typedef TParameterList::const_iterator TParameterListConstIterator;
	TParameterList						ParameterList_;
	BcU32								ParameterBufferSize_;

	typedef std::vector< TUniformBlock > TUniformBlockList;
	typedef TUniformBlockList::iterator TUniformBlockListIterator;
	typedef TUniformBlockList::const_iterator TUniformBlockListConstIterator;
	TUniformBlockList					UniformBlockList_;

	BcU32								NoofShaders_;
	RsShaderGL**						ppShaders_;
};

#endif
