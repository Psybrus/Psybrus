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

#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsProgramGL;

////////////////////////////////////////////////////////////////////////////////
// RsProgramGL
class RsProgramGL:
	public RsProgram
{
public:
	RsProgramGL( RsContext* pContext, BcU32 NoofShaders, RsShader** ppShaders, BcU32 NoofVertexAttributes, RsProgramVertexAttribute* pVertexAttributes );
	virtual ~RsProgramGL();
	
	void								create();
	void								update();
	void								destroy();	
	
	// New, shiny interface!
	virtual BcU32						findSampler( const BcChar* Name );
	virtual void						setSampler( BcU32 Handle, BcU32 SamplerSlotIdx );
	virtual BcU32						findUniformBlockIndex( const BcChar* Name );
	virtual void						setUniformBlock( BcU32 Index, RsUniformBuffer* Buffer );
	virtual void						bind();
	
	virtual const RsProgramVertexAttributeList& getVertexAttributeList() const;
	virtual void logShaders() const;

private:	
	void								addSampler( const GLchar* pName, GLint Handle, GLenum Type );
	void								addBlock( const GLchar* pName, GLint Handle, BcU32 Size );
	
private:
	struct TSampler
	{
		std::string						Name_;
		GLint							Handle_;
		RsShaderParameterType			Type_;
		BcU32							SamplerSlotIdx_;
	};

	struct TUniformBlock
	{
		std::string						Name_;
		GLint							Index_;
		BcU32							Size_;
		RsUniformBuffer*				Buffer_;
	};


	typedef std::vector< TSampler > TSamplerList;
	typedef TSamplerList::iterator TSamplerListIterator;
	typedef TSamplerList::const_iterator TSamplerListConstIterator;
	TSamplerList						SamplerList_;

	typedef std::vector< TUniformBlock > TUniformBlockList;
	typedef TUniformBlockList::iterator TUniformBlockListIterator;
	typedef TUniformBlockList::const_iterator TUniformBlockListConstIterator;
	TUniformBlockList					UniformBlockList_;

	BcU32								NoofShaders_;
	RsShaderGL**						ppShaders_;
	RsProgramVertexAttributeList		AttributeList_;
};

#endif
