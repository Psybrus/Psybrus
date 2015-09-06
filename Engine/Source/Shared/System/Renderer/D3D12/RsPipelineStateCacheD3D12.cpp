#include "System/Renderer/D3D12/RsPipelineStateCacheD3D12.h"
#include "System/Renderer/D3D12/RsUtilsD3D12.h"

#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsShader.h"
#include "System/Renderer/RsVertexDeclaration.h"

#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsFrameBufferFormatDescD3D12::RsFrameBufferFormatDescD3D12():
	NumRenderTargets_( 0 )
{
	RTVFormats_.fill( RsTextureFormat::UNKNOWN );
	DSVFormat_ = RsTextureFormat::UNKNOWN;
}

//////////////////////////////////////////////////////////////////////////
// operator ==
bool RsFrameBufferFormatDescD3D12::operator == ( const RsFrameBufferFormatDescD3D12& Other ) const
{
	return 
		NumRenderTargets_ == Other.NumRenderTargets_ &&
		RTVFormats_ == Other.RTVFormats_ &&
		DSVFormat_ == Other.DSVFormat_;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsGraphicsPipelineStateDescD3D12::RsGraphicsPipelineStateDescD3D12():
	Topology_( RsTopologyType::INVALID ),
	VertexDeclaration_( nullptr ),
	Program_( nullptr ),
	RenderState_( nullptr ),
	FrameBufferFormatDesc_()
{
}

//////////////////////////////////////////////////////////////////////////
// operator == 
bool RsGraphicsPipelineStateDescD3D12::operator == ( const RsGraphicsPipelineStateDescD3D12& Other ) const
{
	return 
		Topology_ == Other.Topology_ &&
		VertexDeclaration_ == Other.VertexDeclaration_ &&
		Program_ == Other.Program_ &&
		RenderState_ == Other.RenderState_ &&
		FrameBufferFormatDesc_ == Other.FrameBufferFormatDesc_;
}

//////////////////////////////////////////////////////////////////////////
// RsGraphicsPipelineStateDescD3D12 hash
namespace std 
{
	size_t hash< RsGraphicsPipelineStateDescD3D12 >::operator()( 
			const RsGraphicsPipelineStateDescD3D12 & PSD ) const
	{
		PSY_PROFILE_FUNCTION;
		BcU32 Hash = 0;
		Hash = BcHash::GenerateCRC32( Hash, &PSD.Topology_, sizeof( PSD.Topology_ ) );
		Hash = BcHash::GenerateCRC32( Hash, &PSD.VertexDeclaration_, sizeof( PSD.VertexDeclaration_ ) );
		Hash = BcHash::GenerateCRC32( Hash, &PSD.Program_, sizeof( PSD.Program_ ) );
		Hash = BcHash::GenerateCRC32( Hash, &PSD.RenderState_, sizeof( PSD.RenderState_ ) );
		Hash = BcHash::GenerateCRC32( Hash, &PSD.FrameBufferFormatDesc_, sizeof( PSD.FrameBufferFormatDesc_ ) );
		return Hash;
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsPipelineStateCacheD3D12::RsPipelineStateCacheD3D12( ID3D12Device* Device ):
	Device_( Device )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsPipelineStateCacheD3D12::~RsPipelineStateCacheD3D12()
{

}

//////////////////////////////////////////////////////////////////////////
// getPipelineState
ID3D12PipelineState* RsPipelineStateCacheD3D12::getPipelineState( 
		const RsGraphicsPipelineStateDescD3D12& GraphicsPSDesc,
		ID3D12RootSignature* RootSignature )
{
	PSY_PROFILE_FUNCTION;
	auto FoundIt = GraphicsPSMap_.find( GraphicsPSDesc );
	if( FoundIt != GraphicsPSMap_.end() )
	{
		return FoundIt->second.Get();
	}

	if( GraphicsPSDesc.Topology_ == RsTopologyType::INVALID ||
		GraphicsPSDesc.VertexDeclaration_ == nullptr ||
		GraphicsPSDesc.Program_ == nullptr ||
		GraphicsPSDesc.RenderState_ == nullptr )
	{
		return nullptr;
	}

	// Setup input elements.
	const auto& VertexDeclDesc = GraphicsPSDesc.VertexDeclaration_->getDesc();
	const auto& VertexAttributes = GraphicsPSDesc.Program_->getVertexAttributeList();

	// Create input layout for current setup.
	// For missing vertex attributes, we just fudge them to be zero offset.
	std::vector< D3D12_INPUT_ELEMENT_DESC > ElementDescs;
	ElementDescs.reserve( VertexDeclDesc.Elements_.size() );

	for( const auto& VertexAttribute : VertexAttributes )
	{
		auto FoundElement = std::find_if( VertexDeclDesc.Elements_.begin(), VertexDeclDesc.Elements_.end(),
			[ & ]( const RsVertexElement& Element )
			{
				return ( Element.Usage_ == VertexAttribute.Usage_ &&
					Element.UsageIdx_ == VertexAttribute.UsageIdx_ );
			} );

		// Force to an element with zero offset if we can't find a valid one.
		// TODO: Find a better approach.
		if( FoundElement == VertexDeclDesc.Elements_.end() )
		{
			FoundElement = std::find_if( VertexDeclDesc.Elements_.begin(), VertexDeclDesc.Elements_.end(),
			[ & ]( const RsVertexElement& Element )
			{
				return Element.Offset_ == 0;
			} );
		}

		D3D12_INPUT_ELEMENT_DESC ElementDesc;
		ElementDesc.SemanticName = RsUtilsD3D12::GetSemanticName( VertexAttribute.Usage_ );
		ElementDesc.SemanticIndex = VertexAttribute.UsageIdx_;
		ElementDesc.Format = RsUtilsD3D12::GetVertexElementFormat( *FoundElement );
		ElementDesc.InputSlot = FoundElement->StreamIdx_;
		ElementDesc.AlignedByteOffset = FoundElement->Offset_;
		ElementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA; // TODO: Instancing support.
		ElementDesc.InstanceDataStepRate = 0;
		ElementDescs.push_back( ElementDesc );
	}
		
	D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc = {};
	BcMemZero( &PSODesc, sizeof( PSODesc ) );

	PSODesc.PrimitiveTopologyType = RsUtilsD3D12::GetPrimitiveTopologyType( GraphicsPSDesc.Topology_ );
	PSODesc.InputLayout.NumElements = static_cast< UINT >( ElementDescs.size() );
	PSODesc.InputLayout.pInputElementDescs = ElementDescs.data();

	const auto & Shaders = GraphicsPSDesc.Program_->getShaders();
	for( const auto * Shader : Shaders )
	{
		auto ShaderDesc = Shader->getDesc();
		switch( ShaderDesc.ShaderType_ )
		{
		case RsShaderType::VERTEX:
			PSODesc.VS.pShaderBytecode = Shader->getData();
			PSODesc.VS.BytecodeLength= Shader->getDataSize();
			break;
		case RsShaderType::PIXEL:
			PSODesc.PS.pShaderBytecode = Shader->getData();
			PSODesc.PS.BytecodeLength= Shader->getDataSize();
			break;
		case RsShaderType::HULL:
			PSODesc.HS.pShaderBytecode = Shader->getData();
			PSODesc.HS.BytecodeLength= Shader->getDataSize();
			break;
		case RsShaderType::DOMAIN:
			PSODesc.DS.pShaderBytecode = Shader->getData();
			PSODesc.DS.BytecodeLength= Shader->getDataSize();
			break;
		case RsShaderType::GEOMETRY:
			PSODesc.GS.pShaderBytecode = Shader->getData();
			PSODesc.GS.BytecodeLength= Shader->getDataSize();
			break;
		}
	}

	PSODesc.pRootSignature = RootSignature;

	{
		// Blend state.
		const auto& Desc = GraphicsPSDesc.RenderState_->getDesc();
		PSODesc.BlendState.AlphaToCoverageEnable = FALSE;
		PSODesc.BlendState.IndependentBlendEnable = TRUE;
		for( size_t Idx = 0; Idx < Desc.BlendState_.RenderTarget_.size(); ++Idx )
		{
			auto SrcBlendState = Desc.BlendState_.RenderTarget_[ Idx ];
			PSODesc.BlendState.RenderTarget[ Idx ].BlendEnable = SrcBlendState.Enable_ ? TRUE : FALSE;
			PSODesc.BlendState.RenderTarget[ Idx ].SrcBlend = RsUtilsD3D12::GetBlend( SrcBlendState.SrcBlend_ );
			PSODesc.BlendState.RenderTarget[ Idx ].DestBlend = RsUtilsD3D12::GetBlend( SrcBlendState.DestBlend_ );
			PSODesc.BlendState.RenderTarget[ Idx ].BlendOp = RsUtilsD3D12::GetBlendOp( SrcBlendState.BlendOp_ );
			PSODesc.BlendState.RenderTarget[ Idx ].SrcBlendAlpha = RsUtilsD3D12::GetBlend( SrcBlendState.SrcBlendAlpha_ );
			PSODesc.BlendState.RenderTarget[ Idx ].DestBlendAlpha = RsUtilsD3D12::GetBlend( SrcBlendState.DestBlendAlpha_ );
			PSODesc.BlendState.RenderTarget[ Idx ].BlendOpAlpha = RsUtilsD3D12::GetBlendOp( SrcBlendState.BlendOpAlpha_ );
			PSODesc.BlendState.RenderTarget[ Idx ].RenderTargetWriteMask = SrcBlendState.WriteMask_;
		}

		// Rasterizer state.
		auto SrcRasterizerState = Desc.RasteriserState_;
		PSODesc.RasterizerState.FillMode = RsUtilsD3D12::GetFillMode( SrcRasterizerState.FillMode_ );
		PSODesc.RasterizerState.CullMode = RsUtilsD3D12::GetCullMode( SrcRasterizerState.CullMode_ );
		PSODesc.RasterizerState.FrontCounterClockwise = FALSE;
		PSODesc.RasterizerState.DepthBias = (INT)SrcRasterizerState.DepthBias_;
		PSODesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		PSODesc.RasterizerState.SlopeScaledDepthBias = SrcRasterizerState.SlopeScaledDepthBias_;
		PSODesc.RasterizerState.DepthClipEnable = SrcRasterizerState.DepthClipEnable_ ? TRUE : FALSE;
		PSODesc.RasterizerState.MultisampleEnable = FALSE;
		//PSODesc.RasterizerState.ScissorEnable = SrcRasterizerState.ScissorEnable_ ? TRUE : FALSE;
		PSODesc.RasterizerState.AntialiasedLineEnable = SrcRasterizerState.AntialiasedLineEnable_ ? TRUE : FALSE;
		PSODesc.RasterizerState.ForcedSampleCount = 0;
		PSODesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// Depth stencil state.
		auto SrcDepthStencilState = Desc.DepthStencilState_;
		PSODesc.DepthStencilState.DepthEnable = SrcDepthStencilState.DepthTestEnable_ ? TRUE : FALSE;
		PSODesc.DepthStencilState.DepthWriteMask = SrcDepthStencilState.DepthWriteEnable_ ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		PSODesc.DepthStencilState.DepthFunc = RsUtilsD3D12::GetComparisonFunc( SrcDepthStencilState.DepthFunc_ );
		PSODesc.DepthStencilState.StencilEnable = SrcDepthStencilState.StencilEnable_ ? TRUE : FALSE;
		PSODesc.DepthStencilState.StencilReadMask = SrcDepthStencilState.StencilRead_;
		PSODesc.DepthStencilState.StencilWriteMask = SrcDepthStencilState.StencilWrite_;
		PSODesc.DepthStencilState.FrontFace.StencilFailOp = RsUtilsD3D12::GetStencilOp( SrcDepthStencilState.StencilFront_.Fail_ );
		PSODesc.DepthStencilState.FrontFace.StencilDepthFailOp = RsUtilsD3D12::GetStencilOp( SrcDepthStencilState.StencilFront_.DepthFail_ );
		PSODesc.DepthStencilState.FrontFace.StencilPassOp = RsUtilsD3D12::GetStencilOp( SrcDepthStencilState.StencilFront_.Pass_ );
		PSODesc.DepthStencilState.FrontFace.StencilFunc = RsUtilsD3D12::GetComparisonFunc( SrcDepthStencilState.StencilFront_.Func_ );
		PSODesc.DepthStencilState.BackFace.StencilFailOp = RsUtilsD3D12::GetStencilOp( SrcDepthStencilState.StencilBack_.Fail_ );
		PSODesc.DepthStencilState.BackFace.StencilDepthFailOp = RsUtilsD3D12::GetStencilOp( SrcDepthStencilState.StencilBack_.DepthFail_ );
		PSODesc.DepthStencilState.BackFace.StencilPassOp = RsUtilsD3D12::GetStencilOp( SrcDepthStencilState.StencilBack_.Pass_ );
		PSODesc.DepthStencilState.BackFace.StencilFunc = RsUtilsD3D12::GetComparisonFunc( SrcDepthStencilState.StencilBack_.Func_ );
	}

	PSODesc.NumRenderTargets = static_cast< UINT >( GraphicsPSDesc.FrameBufferFormatDesc_.NumRenderTargets_ );
	for( size_t Idx = 0; Idx < 8; ++Idx )
	{
		PSODesc.RTVFormats[ Idx ] = 
			RsUtilsD3D12::GetTextureFormat( GraphicsPSDesc.FrameBufferFormatDesc_.RTVFormats_[ Idx ] ).RTVFormat_;
	}
	PSODesc.DSVFormat = 
		RsUtilsD3D12::GetTextureFormat( GraphicsPSDesc.FrameBufferFormatDesc_.DSVFormat_ ).DSVFormat_;

	PSODesc.SampleDesc.Count = 1;
	PSODesc.SampleDesc.Quality = 0;
	PSODesc.SampleMask = UINT_MAX;

	// Construct a new graphics pipeline state.
	ComPtr< ID3D12PipelineState > GraphicsPS;

	HRESULT RetVal = Device_->CreateGraphicsPipelineState( &PSODesc, IID_PPV_ARGS( GraphicsPS.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );
		
	// Add to map & return.
	GraphicsPSMap_.insert( std::make_pair( GraphicsPSDesc, GraphicsPS ) );
	return GraphicsPS.Get();
}

//////////////////////////////////////////////////////////////////////////
// destroyResources
void RsPipelineStateCacheD3D12::destroyResources( ShouldDestroyFunc DestroyFunc )
{
	PSY_PROFILE_FUNCTION;
	for( auto It = GraphicsPSMap_.begin(); It != GraphicsPSMap_.end(); )
	{
		auto ShouldDestroy = DestroyFunc( It->first, It->second.Get() );
		if( ShouldDestroy )
		{
			It = GraphicsPSMap_.erase( It );
		}
		else
		{
			++It;
		}
	}
}

