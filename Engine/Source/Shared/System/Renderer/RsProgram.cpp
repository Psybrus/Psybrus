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
		RsProgramVertexAttributeList&& VertexAttributes,
		RsProgramUniformList&& UniformList,
		RsProgramUniformBlockList&& UniformBlockList ):
	RsResource( pContext ),
	Shaders_( std::move( Shaders ) ),
	AttributeList_( std::move( VertexAttributes ) ),
	UniformList_( std::move( UniformList ) ),
	UniformBlockList_( std::move( UniformBlockList ) )
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
	for( const auto& It : SamplerList_ )
	{
		if( It.Name_ == Name )
		{
			return It.Handle_;
		}
	}

	return BcErrorCode;
}

////////////////////////////////////////////////////////////////////////////////
// findTextureSlot
BcU32 RsProgram::findTextureSlot( const BcChar* Name )
{
	for( const auto& It : TextureList_ )
	{
		if( It.Name_ == Name )
		{
			return It.Handle_;
		}
	}

	return BcErrorCode;
}

////////////////////////////////////////////////////////////////////////////////
// findUniformBufferSlot
BcU32 RsProgram::findUniformBufferSlot( const BcChar* Name )
{
	for( const auto& It : InternalUniformBlockList_ )
	{
		if( It.Name_ == Name )
		{
			return It.Handle_;
		}
	}

	return BcErrorCode;
}

////////////////////////////////////////////////////////////////////////////////
// getUniformBufferClass
const ReClass* RsProgram::getUniformBufferClass( BcU32 Handle )
{
	for( const auto& It : InternalUniformBlockList_ )
	{
		if( It.Handle_ == Handle )
		{
			return It.Class_;
		}
	}

	return nullptr;
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
void RsProgram::addUniformBufferSlot( std::string Name, BcU32 Handle, const ReClass* Class )
{
	// Must have a valid class.
	BcAssert( Class != nullptr );

	TUniformBlock Block = 
	{
		std::move( Name ),
		Handle,
		Class
	};

	InternalUniformBlockList_.push_back( std::move( Block ) );
}
