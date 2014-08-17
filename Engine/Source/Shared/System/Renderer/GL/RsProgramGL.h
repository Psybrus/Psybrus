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

#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsShader.h"

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
		RsProgramVertexAttributeList&& VertexAttributes );
	virtual ~RsProgramGL();
	
	void create();
	void update();
	void destroy();	
	
	// New, shiny interface!
	virtual BcU32 findSamplerSlot( const BcChar* Name );
	virtual BcU32 findUniformBufferSlot( const BcChar* Name );
	
	virtual const RsProgramVertexAttributeList& getVertexAttributeList() const;
	virtual void logShaders() const;

public:
	// Used internally by the renderer to patch reflection information
	// into the program.
	void addSampler( std::string Name, BcU32 Handle, RsShaderParameterType Type );
	void addBlock( std::string Name, BcU32 Handle, BcU32 Size );
	
private:
	struct TSampler
	{
		std::string Name_;
		BcU32 Handle_;
		RsShaderParameterType Type_;
	};

	struct TUniformBlock
	{
		std::string Name_;
		BcU32 Index_;
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
