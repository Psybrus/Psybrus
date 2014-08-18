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
#include "System/Renderer/RsShader.h"

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
	// Find vertex shader.
	BcU64 HashCalc = 0;
	for( auto* Shader : Shaders_ )
	{
		if( Shader->getDesc().ShaderType_ == RsShaderType::VERTEX )
		{
			HashCalc = Shader->getHandle< BcU64 >();
		}
	}

	// Hash input layout + vertex shader.
	InputLayoutHash_ = BcHash::GenerateCRC32( 0, &HashCalc, sizeof( HashCalc ) );
	InputLayoutHash_ = BcHash::GenerateCRC32( InputLayoutHash_, &AttributeList_[ 0 ], sizeof( RsProgramVertexAttribute ) * AttributeList_.size() );
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
// findTextureSlot
BcU32 RsProgram::findTextureSlot( const BcChar* Name )
{
	BcU32 Idx = 0;
	for( auto It( TextureList_.begin() ); It != TextureList_.end(); ++It )
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
// getInputLayoutHash
BcU32 RsProgram::getInputLayoutHash() const
{
	return InputLayoutHash_;
}

////////////////////////////////////////////////////////////////////////////////
// addSamplerSlot
void RsProgram::addSamplerSlot( std::string Name, BcU32 Handle )
{
	// If parameter is valid, add it.
	TSampler Sampler = 
	{
		std::move( Name ),
		Handle
	};
	SamplerList_.push_back( std::move( Sampler ) );
}

////////////////////////////////////////////////////////////////////////////////
// addTextureSlot
void RsProgram::addTextureSlot( std::string Name, BcU32 Handle )
{
	// If parameter is valid, add it.
	TTexture Texture = 
	{
		std::move( Name ),
		Handle,
	};
	TextureList_.push_back( std::move( Texture ) );
}

////////////////////////////////////////////////////////////////////////////////
// addUniformBufferSlot
void RsProgram::addUniformBufferSlot( std::string Name, BcU32 Handle, BcU32 Size )
{
	TUniformBlock Block = 
	{
		std::move( Name ),
		Handle,
		Size
	};

	UniformBlockList_.push_back( std::move( Block ) );
}

