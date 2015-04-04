#include "System/Renderer/D3D12/RsPipelineStateCacheD3D12.h"

#include "System/Renderer/RsShader.h"


//////////////////////////////////////////////////////////////////////////
// Ctor
RsFrameBufferFormatDescD3D12::RsFrameBufferFormatDescD3D12():
	NumRenderTargets_( 0 )
{
	RTVFormats_.fill( RsTextureFormat::R8G8B8A8 );
	DSVFormat_ = RsTextureFormat::D24S8;
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
	size_t hash< RsGraphicsPipelineStateDescD3D12 >::operator()( const RsGraphicsPipelineStateDescD3D12 & PSD ) const
	{
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
ID3D12PipelineState* RsPipelineStateCacheD3D12::getPipelineState( const RsGraphicsPipelineStateDescD3D12& GraphicsPSDesc )
{
	auto FoundIt = GraphicsPSMap_.find( GraphicsPSDesc );
	if( FoundIt != GraphicsPSMap_.end() )
	{
		return FoundIt->second.Get();
	}

	// Construct a new graphics pipeline state.
	ComPtr< ID3D12PipelineState > GraphicsPS;
	
	
	
	// Add to map & return.
	GraphicsPSMap_.insert( std::make_pair( GraphicsPSDesc, GraphicsPS ) );
	return GraphicsPS.Get();
}