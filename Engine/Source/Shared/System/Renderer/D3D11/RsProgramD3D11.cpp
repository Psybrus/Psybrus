#include "System/Renderer/D3D11/RsProgramD3D11.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsShader.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramD3D11::RsProgramD3D11( class RsProgram* Parent, ID3D11Device* Device ):
	Parent_( Parent ),
	Device_( Device )
{
	// TODO: Look up also by type, size, and flags. Not just name.
	// TODO: Do this work offline.
	typedef std::map< std::string, SlotMapping > ResourceHandleMapping;
	typedef std::map< std::string, BcU32 > ResourceSizeMapping;
	ResourceHandleMapping SamplerBindings;
	ResourceHandleMapping SRVBindings;
	ResourceHandleMapping UAVBindings;
	ResourceHandleMapping CBBindings;
	ResourceSizeMapping CBSizes;

	// Iterate over shaders and setup handles for all constant
	// buffers and shader resources.
	for( auto* Shader : Parent_->getShaders() )
	{
		const auto& ShaderDesc = Shader->getDesc();
		BcU32 ShaderTypeIdx = static_cast< BcU32 >( ShaderDesc.ShaderType_ );
		ID3D11ShaderReflection* Reflector = nullptr; 
		D3DReflect( Shader->getData(), Shader->getDataSize(),
			IID_ID3D11ShaderReflection, (void**)&Reflector );

		// Just iterate over a big number...we'll assert if we go over.
		for( BcU32 Idx = 0; Idx < 128; ++Idx )
		{
			D3D11_SHADER_INPUT_BIND_DESC BindDesc;
			if( SUCCEEDED( Reflector->GetResourceBindingDesc( Idx, &BindDesc ) ) )
			{
				// Check if it's a cbuffer or tbuffer.
				if( BindDesc.Type == D3D_SIT_CBUFFER || 
					BindDesc.Type == D3D_SIT_TBUFFER )
				{
					if( CBBindings.find( BindDesc.Name ) == CBBindings.end() )
					{
						CBBindings[ BindDesc.Name ] = SlotMapping();
					}

					SlotMapping& SlotMapping = CBBindings[ BindDesc.Name ];
					BcU32 Size = CBSizes[ BindDesc.Name ];			
					
					auto ConstantBuffer = Reflector->GetConstantBufferByName( BindDesc.Name );
					D3D11_SHADER_BUFFER_DESC BufferDesc;
					ConstantBuffer->GetDesc( &BufferDesc );
					if( Size != 0 )
					{
						BcAssert( BufferDesc.Size == Size );
					}

					SlotMapping.ShaderSlot[ ShaderTypeIdx ] = BindDesc.BindPoint;
					Size = BufferDesc.Size;
					CBSizes[ BindDesc.Name ] = Size;
				}
				else if( 
					BindDesc.Type == D3D_SIT_TEXTURE ||
					BindDesc.Type == D3D_SIT_STRUCTURED || 
					BindDesc.Type == D3D_SIT_BYTEADDRESS )
				{
					if( SRVBindings.find( BindDesc.Name ) == SRVBindings.end() )
					{
						SRVBindings[ BindDesc.Name ] = SlotMapping();
					}

					SlotMapping& SlotMapping = SRVBindings[ BindDesc.Name ];
					SlotMapping.ShaderSlot[ ShaderTypeIdx ] = BindDesc.BindPoint;
				}
				else if( BindDesc.Type == D3D_SIT_UAV_RWTYPED ||
					BindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED || 
					BindDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS ||
					BindDesc.Type == D3D_SIT_UAV_APPEND_STRUCTURED || 
					BindDesc.Type == D3D_SIT_UAV_CONSUME_STRUCTURED || 
					BindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER )
				{
					if( UAVBindings.find( BindDesc.Name ) == UAVBindings.end() )
					{
						UAVBindings[ BindDesc.Name ] = SlotMapping();
					}

					SlotMapping& SlotMapping = UAVBindings[ BindDesc.Name ];
					SlotMapping.ShaderSlot[ ShaderTypeIdx ] = BindDesc.BindPoint;
				}
				else if( BindDesc.Type == D3D_SIT_SAMPLER )
				{
					if( SamplerBindings.find( BindDesc.Name ) == SamplerBindings.end() )
					{
						SamplerBindings[ BindDesc.Name ] = SlotMapping();
					}

					SlotMapping& SlotMapping = SamplerBindings[ BindDesc.Name ];
					SlotMapping.ShaderSlot[ ShaderTypeIdx ] = BindDesc.BindPoint;
				}
				else
				{
					BcBreakpoint;
				}
			}
		}
	}

	// Add all sampler bindings
	BcU32 SamplerIdx = 0;
	for( const auto& Sampler : SamplerBindings )
	{
		// TEMPORARY HACK.
		std::string SamplerName = Sampler.first;
		if( SamplerName[0] == 's' )
		{
			SamplerName[0] = 'a';
		}

		Parent_->addSamplerSlot( 
			SamplerName,
			SamplerIdx );
		SamplerSlots_[ SamplerIdx++ ] = Sampler.second;
	}

	// Add all SRV bindings
	BcU32 SRVIdx = 0;
	for( const auto& SRV : SRVBindings )
	{
		Parent_->addShaderResource( 
			SRV.first,
			RsShaderResourceType::TEXTURE,
			SRVIdx );
		SRVSlots_[ SRVIdx++ ] = SRV.second;
	}

	// Add all UAV bindings
	BcU32 UAVIdx = 0;
	for( const auto& UAV : UAVBindings )
	{
		Parent_->addUnorderedAccess( 
			UAV.first,
			RsUnorderedAccessType::BUFFER,
			UAVIdx );
		UAVSlots_[ UAVIdx++ ] = UAV.second;
	}

	// Add all cb bindings
	BcU32 CBIdx = 0;
	for( const auto& CB : CBBindings )
	{
		auto Size = CBSizes[ CB.first ];
		auto Class = ReManager::GetClass( CB.first );
		BcAssert( Class->getSize() == Size );
		Parent_->addUniformBufferSlot(
			CB.first,
			CBIdx,
			Class );
		CBSlots_[ CBIdx++ ] = CB.second;
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsProgramD3D11::~RsProgramD3D11()
{
}

//////////////////////////////////////////////////////////////////////////
// getSamplerSlot
BcU32 RsProgramD3D11::getSamplerSlot( RsShaderType Type, BcU32 Idx )
{
	return SamplerSlots_[ Idx ].ShaderSlot[ (BcU32)Type ];
}

//////////////////////////////////////////////////////////////////////////
// getSRVSlot
BcU32 RsProgramD3D11::getSRVSlot( RsShaderType Type, BcU32 Idx )
{
	return SRVSlots_[ Idx ].ShaderSlot[ (BcU32)Type ];
}

//////////////////////////////////////////////////////////////////////////
// getUAVSlot
BcU32 RsProgramD3D11::getUAVSlot( RsShaderType Type, BcU32 Idx )
{
	return UAVSlots_[ Idx ].ShaderSlot[ (BcU32)Type ];
}

//////////////////////////////////////////////////////////////////////////
// getCBSlot
BcU32 RsProgramD3D11::getCBSlot( RsShaderType Type, BcU32 Idx )
{
	return CBSlots_[ Idx ].ShaderSlot[ (BcU32)Type ];
}
