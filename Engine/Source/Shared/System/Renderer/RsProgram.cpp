/**************************************************************************
*
* File:		RsProgram.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsProgram.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsProgram::RsProgram( 
	class RsContext* pContext, 
	std::vector< RsShader* >&& Shaders, 
	RsProgramVertexAttributeList&& VertexAttributes ):
	RsResource( pContext ),
	Shaders_( std::move( Shaders ) ),
	AttributeList_( std::move( VertexAttributes ) )
{

}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtua
RsProgram::~RsProgram()
{

}

////////////////////////////////////////////////////////////////////////////////
// findSamplerSlot
BcU32 RsProgram::findSamplerSlot( const BcChar* Name )
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
BcU32 RsProgram::findUniformBufferSlot( const BcChar* Name )
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
// getShaders
const std::vector< class RsShader* >& RsProgram::getShaders() const
{
	return Shaders_;
}

////////////////////////////////////////////////////////////////////////////////
// addParameter
const RsProgramVertexAttributeList& RsProgram::getVertexAttributeList() const
{
	return AttributeList_;
}

////////////////////////////////////////////////////////////////////////////////
// addParameter
void RsProgram::addSampler( std::string Name, BcU32 Handle, RsShaderParameterType Type )
{
	// If parameter is valid, add it.
	TSampler Sampler = 
	{
		std::move( Name ),
		Handle,
		Type,
	};
	SamplerList_.push_back( std::move( Sampler ) );
}

////////////////////////////////////////////////////////////////////////////////
// addBlock
void RsProgram::addBlock( std::string Name, BcU32 Handle, BcU32 Size )
{
	TUniformBlock Block = 
	{
		std::move( Name ),
		Handle,
		Size
	};

	UniformBlockList_.push_back( std::move( Block ) );
}

