/**************************************************************************
*
* File:		RsProgram.h
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSPROGRAM_H__
#define __RSPROGRAM_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

////////////////////////////////////////////////////////////////////////////////
// RsProgram
class RsProgram:
	public RsResource
{
public:
	RsProgram( 
		class RsContext* pContext, 
		std::vector< class RsShader* >&& Shaders,
		RsProgramVertexAttributeList&& VertexAttributes );
	virtual ~RsProgram();
	
	BcU32 findSamplerSlot( const BcChar* Name );
	BcU32 findTextureSlot( const BcChar* Name );
	BcU32 findUniformBufferSlot( const BcChar* Name );

	const std::vector< class RsShader* >& getShaders() const;
	const RsProgramVertexAttributeList& getVertexAttributeList() const;

public:
	// Used internally by the renderer to patch reflection information
	// into the program.
	void addSamplerSlot( std::string Name, BcU32 Handle );
	void addTextureSlot( std::string Name, BcU32 Handle );
	void addUniformBufferSlot( std::string Name, BcU32 Handle, BcU32 Size );

private:
	struct TSampler
	{
		std::string Name_;
		BcU32 Handle_;
	};

	struct TTexture
	{
		std::string Name_;
		BcU32 Handle_;
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

	typedef std::vector< TTexture > TTextureList;
	typedef TTextureList::iterator TTextureListIterator;
	typedef TTextureList::const_iterator TTextureListConstIterator;
	TTextureList TextureList_;

	typedef std::vector< TUniformBlock > TUniformBlockList;
	typedef TUniformBlockList::iterator TUniformBlockListIterator;
	typedef TUniformBlockList::const_iterator TUniformBlockListConstIterator;
	TUniformBlockList UniformBlockList_;

	std::vector< class RsShader* > Shaders_;
	RsProgramVertexAttributeList AttributeList_;

};

#endif
