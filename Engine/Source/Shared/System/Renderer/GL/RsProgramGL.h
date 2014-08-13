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
	RsProgramGL( 
		RsContext* pContext, 
		std::vector< RsShader* >&& Shaders,
		BcU32 NoofVertexAttributes, 
		RsProgramVertexAttribute* pVertexAttributes );
	virtual ~RsProgramGL();
	
	void create();
	void update();
	void destroy();	
	
	// New, shiny interface!
	virtual BcU32 findSamplerSlot( const BcChar* Name );
	virtual BcU32 findUniformBufferSlot( const BcChar* Name );
	virtual void bind();
	
	virtual const RsProgramVertexAttributeList& getVertexAttributeList() const;
	virtual void logShaders() const;

private:	
	void addSampler( const GLchar* pName, GLint Handle, GLenum Type );
	void addBlock( const GLchar* pName, GLint Handle, BcU32 Size );
	
private:
	struct TSampler
	{
		std::string Name_;
		GLint Handle_;
		RsShaderParameterType Type_;
	};

	struct TUniformBlock
	{
		std::string Name_;
		GLint Index_;
		BcU32 Size_;
	};

	typedef std::vector< TSampler > TSamplerList;
	typedef TSamplerList::iterator TSamplerListIterator;
	typedef TSamplerList::const_iterator TSamplerListConstIterator;
	TSamplerList SamplerList_;

	typedef std::vector< TUniformBlock > TUniformBlockList;
	typedef TUniformBlockList::iterator TUniformBlockListIterator;
	typedef TUniformBlockList::const_iterator TUniformBlockListConstIterator;
	TUniformBlockList UniformBlockList_;

	std::vector< RsShader* > Shaders_;
	RsProgramVertexAttributeList AttributeList_;
};

#endif
