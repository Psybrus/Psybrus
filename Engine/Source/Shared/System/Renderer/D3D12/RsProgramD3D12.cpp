#include "System/Renderer/D3D12/RsProgramD3D12.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsShader.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramD3D12::RsProgramD3D12( class RsProgram* Parent, ID3D12Device* Device ):
	Parent_( Parent ),
	Device_( Device )
{
	// TODO: Look up also by type, size, and flags. Not just name.
	// TODO: Do this work offline.
	typedef std::map< std::string, BcU32 > ResourceHandleMapping;
	ResourceHandleMapping SamplerBindings;
	ResourceHandleMapping TextureBindings;
	ResourceHandleMapping BufferBindings;
	ResourceHandleMapping ConstantBufferBindings;
	ResourceHandleMapping ConstantBufferSizes;

	// Iterate over shaders and setup handles for all constant
	// buffers and shader resources.
	for( auto* Shader : Parent_->getShaders() )
	{
		const auto& ShaderDesc = Shader->getDesc();
		ID3D12ShaderReflection* Reflector = nullptr; 
		D3DReflect( Shader->getData(), Shader->getDataSize(),
			IID_ID3D12ShaderReflection, (void**)&Reflector );

		const BcU32 ShiftAmount = ( (BcU32)ShaderDesc.ShaderType_ * BitsPerShader );
		const BcU32 MaskOff = ~( MaxBindPoints << ShiftAmount );

		// Just iterate over a big number...we'll assert if we go over.
		for( BcU32 Idx = 0; Idx < 128; ++Idx )
		{
			D3D12_SHADER_INPUT_BIND_DESC BindDesc;
			if( SUCCEEDED( Reflector->GetResourceBindingDesc( Idx, &BindDesc ) ) )
			{
				// Validate.
				BcAssert( 
					BindDesc.BindPoint < MaxBindPoints && 
					( BindDesc.BindPoint + BindDesc.BindCount ) <= MaxBindPoints );

				// Check if it's a cbuffer or tbuffer.
				if( BindDesc.Type == D3D_SIT_CBUFFER || 
					BindDesc.Type == D3D_SIT_TBUFFER )
				{
					if( ConstantBufferBindings.find( BindDesc.Name ) == ConstantBufferBindings.end() )
					{
						ConstantBufferBindings[ BindDesc.Name ] = BcErrorCode;
					}

					BcU32 Handle = ConstantBufferBindings[ BindDesc.Name ];
					BcU32 Size = ConstantBufferSizes[ BindDesc.Name ];			
					
					auto ConstantBuffer = Reflector->GetConstantBufferByName( BindDesc.Name );
					D3D12_SHADER_BUFFER_DESC BufferDesc;
					ConstantBuffer->GetDesc( &BufferDesc );
					if( Size != 0 )
					{
						BcAssert( BufferDesc.Size == Size );
					}

					Handle = ( Handle & MaskOff ) | ( BindDesc.BindPoint << ShiftAmount );
					Size = BufferDesc.Size;
					ConstantBufferBindings[ BindDesc.Name ] = Handle;
					ConstantBufferSizes[ BindDesc.Name ] = Size;
				}
				else if( BindDesc.Type == D3D_SIT_TEXTURE )
				{
					if( TextureBindings.find( BindDesc.Name ) == TextureBindings.end() )
					{
						TextureBindings[ BindDesc.Name ] = BcErrorCode;
					}

					BcU32 Handle = TextureBindings[ BindDesc.Name ];
					Handle = ( Handle & MaskOff ) | ( BindDesc.BindPoint << ShiftAmount );
					TextureBindings[ BindDesc.Name ] = Handle;
				}
				else if( 
					BindDesc.Type == D3D_SIT_TEXTURE ||
					BindDesc.Type == D3D_SIT_STRUCTURED || 
					BindDesc.Type == D3D_SIT_BYTEADDRESS )
				{
					if( BufferBindings.find( BindDesc.Name ) == BufferBindings.end() )
					{
						BufferBindings[ BindDesc.Name ] = BcErrorCode;
					}

					BcU32 Handle = BufferBindings[ BindDesc.Name ];
					Handle = ( Handle & MaskOff ) | ( BindDesc.BindPoint << ShiftAmount );
					BufferBindings[ BindDesc.Name ] = Handle;
				}
				else if( BindDesc.Type == D3D_SIT_UAV_RWTYPED ||
					BindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED || 
					BindDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS ||
					BindDesc.Type == D3D_SIT_UAV_APPEND_STRUCTURED || 
					BindDesc.Type == D3D_SIT_UAV_CONSUME_STRUCTURED || 
					BindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER )
				{
					if( BufferBindings.find( BindDesc.Name ) == BufferBindings.end() )
					{
						BufferBindings[ BindDesc.Name ] = BcErrorCode;
					}

					BcU32 Handle = BufferBindings[ BindDesc.Name ];
					Handle = ( Handle & MaskOff ) | ( BindDesc.BindPoint << ShiftAmount );
					BufferBindings[ BindDesc.Name ] = Handle;
				}
				else if( BindDesc.Type == D3D_SIT_SAMPLER )
				{
					if( SamplerBindings.find( BindDesc.Name ) == SamplerBindings.end() )
					{
						SamplerBindings[ BindDesc.Name ] = BcErrorCode;
					}

					BcU32 Handle = SamplerBindings[ BindDesc.Name ];
					Handle = ( Handle & MaskOff ) | ( BindDesc.BindPoint << ShiftAmount );
					SamplerBindings[ BindDesc.Name ] = Handle;
				}
				else
				{
					BcBreakpoint;
				}
			}
		}
	}

	// Add all constant buffer bindings
	for( const auto& ConstantBuffer : ConstantBufferBindings )
	{
		auto Size = ConstantBufferSizes[ ConstantBuffer.first ];
		auto Class = ReManager::GetClass( ConstantBuffer.first );
		BcAssert( Class->getSize() == Size );
		Parent_->addUniformBufferSlot( 
			ConstantBuffer.first,
			ConstantBuffer.second,
			Class );
	}

	// Add all sampler bindings
	for( const auto& Sampler : SamplerBindings )
	{
		// TEMPORARY HACK.
		std::string SamplerName = Sampler.first;
		if( SamplerName[0] == 's' )
		{
			SamplerName[0] = 'a';
		}

		Parent_->addSamplerSlot( 
			Sampler.first,
			Sampler.second );
	}

	// Add all texture bindings
	for( const auto& Texture : TextureBindings )
	{
		Parent_->addShaderResource( 
			Texture.first,
			RsShaderResourceType::TEXTURE,
			Texture.second );
	}

	// Add all buffer bindings
	for( const auto& Buffer : BufferBindings )
	{
		Parent_->addShaderResource( 
			Buffer.first,
			RsShaderResourceType::BUFFER,
			Buffer.second );
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsProgramD3D12::~RsProgramD3D12()
{
}

