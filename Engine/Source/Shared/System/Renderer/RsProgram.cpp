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
		RsProgramParameterList&& ParameterList ):
	RsResource( RsResourceType::PROGRAM, pContext ),
	Shaders_( std::move( Shaders ) ),
	AttributeList_( std::move( VertexAttributes ) ),
	ParameterList_( std::move( ParameterList ) )
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
	InputLayoutHash_ = BcHash::GenerateCRC32( InputLayoutHash_, AttributeList_.data(), sizeof( RsProgramVertexAttribute ) * AttributeList_.size() );
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtua
RsProgram::~RsProgram()
{

}

////////////////////////////////////////////////////////////////////////////////
// findSamplerSlot
BcU32 RsProgram::findSamplerSlot( const BcChar* Name ) const
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
// findShaderResourceSlot
BcU32 RsProgram::findShaderResourceSlot( const BcChar* Name ) const
{
	for( const auto& It : ShaderResourceList_ )
	{
		if( It.Name_ == Name )
		{
			return It.Handle_;
		}
	}

	return BcErrorCode;
}

////////////////////////////////////////////////////////////////////////////////
// findUnorderedAccessSlot
BcU32 RsProgram::findUnorderedAccessSlot( const BcChar* Name ) const
{
	for( const auto& It : UnorderedAccessList_ )
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
BcU32 RsProgram::findUniformBufferSlot( const BcChar* Name ) const
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
// getSamplerSlotName
const char* RsProgram::getSamplerSlotName( BcU32 Slot ) const
{
	BcAssert( SamplerList_[ Slot ].Handle_ == Slot );
	return SamplerList_[ Slot ].Name_.c_str();
}

////////////////////////////////////////////////////////////////////////////////
// getShaderResourceSlotName
const char* RsProgram::getShaderResourceSlotName( BcU32 Slot ) const
{
	BcAssert( ShaderResourceList_[ Slot ].Handle_ == Slot );
	return ShaderResourceList_[ Slot ].Name_.c_str();
}

////////////////////////////////////////////////////////////////////////////////
// getUnorderedAccessSlotName
const char* RsProgram::getUnorderedAccessSlotName( BcU32 Slot ) const
{
	BcAssert( UnorderedAccessList_[ Slot ].Handle_ == Slot );
	return UnorderedAccessList_[ Slot ].Name_.c_str();
}

////////////////////////////////////////////////////////////////////////////////
// getUniformBufferSlotName
const char* RsProgram::getUniformBufferSlotName( BcU32 Slot ) const
{
	BcAssert( InternalUniformBlockList_[ Slot ].Handle_ == Slot );
	return InternalUniformBlockList_[ Slot ].Name_.c_str();
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
// getParameterList
const RsProgramParameterList& RsProgram::getParameterList() const
{
	return ParameterList_;
}

////////////////////////////////////////////////////////////////////////////////
// getInputLayoutHash
BcU32 RsProgram::getInputLayoutHash() const
{
	return InputLayoutHash_;
}

////////////////////////////////////////////////////////////////////////////////
// isGraphics
bool RsProgram::isGraphics() const
{
	return !isCompute();
}

////////////////////////////////////////////////////////////////////////////////
// isCompute
bool RsProgram::isCompute() const
{
	return Shaders_[ 0 ]->getDesc().ShaderType_ == RsShaderType::COMPUTE;
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
// addShaderResource
void RsProgram::addShaderResource( std::string Name, RsShaderResourceType Type, BcU32 Handle )
{
	// If parameter is valid, add it.
	TShaderResource ShaderResource = 
	{
		std::move( Name ),
		Type,
		Handle,
	};
	ShaderResourceList_.push_back( std::move( ShaderResource ) );
}

////////////////////////////////////////////////////////////////////////////////
// addUnorderedAccess
void RsProgram::addUnorderedAccess( std::string Name, RsUnorderedAccessType Type, BcU32 Handle )
{
	// If parameter is valid, add it.
	TUnorderedAccess UnorderedAccess = 
	{
		std::move( Name ),
		Type,
		Handle,
	};
	UnorderedAccessList_.push_back( std::move( UnorderedAccess ) );
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

	InternalUniformBlockList_.push_back( std::move( Block ) );
}
