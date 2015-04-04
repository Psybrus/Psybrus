#include "System/Renderer/D3D12/RsResourceD3D12.h"

#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsTexture.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsResourceD3D12::RsResourceD3D12( ID3D12Resource* Resource, RsResourceBindFlags BindFlags, RsResourceBindFlags InitialBindType ):
	Resource_( Resource ),
	BindFlags_( BindFlags ),
	CurrentBindType_( InitialBindType )
{
	BcAssert( ( InitialBindType & BindFlags_ ) != RsResourceBindFlags::NONE );
	BcAssert( BcBitsSet( (BcU32)InitialBindType ) == 1 );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsResourceD3D12::~RsResourceD3D12()
{
}

//////////////////////////////////////////////////////////////////////////
// getInternalResource
ComPtr< ID3D12Resource >& RsResourceD3D12::getInternalResource()
{
	return Resource_;
}

//////////////////////////////////////////////////////////////////////////
// resourceBarrierTransition
void RsResourceD3D12::resourceBarrierTransition( ID3D12GraphicsCommandList* CommandList, RsResourceBindFlags BindType )
{
	BcAssert( ( BindType & BindFlags_ ) != RsResourceBindFlags::NONE );
	BcAssert( BcBitsSet( (BcU32)BindType ) == 1 );

	D3D12_RESOURCE_BARRIER_DESC descBarrier = {};
	descBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	descBarrier.Transition.pResource = Resource_.Get();
	descBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	descBarrier.Transition.StateBefore = RsResourceD3D12::GetResourceUsage( CurrentBindType_ );
	descBarrier.Transition.StateAfter = RsResourceD3D12::GetResourceUsage( BindType );

	CommandList->ResourceBarrier( 1, &descBarrier );
	CurrentBindType_ = BindType;
}

//////////////////////////////////////////////////////////////////////////
// resourceBarrierTransition
D3D12_RESOURCE_USAGE RsResourceD3D12::GetResourceUsage( RsResourceBindFlags BindFlags )
{
	BcU32 AllowedUsage = 0;
	if( ( BindFlags & RsResourceBindFlags::VERTEX_BUFFER ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_GENERIC_READ;
	}
	if( ( BindFlags & RsResourceBindFlags::INDEX_BUFFER ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_GENERIC_READ;
	}
	if( ( BindFlags & RsResourceBindFlags::UNIFORM_BUFFER ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_GENERIC_READ;
	}
	if( ( BindFlags & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_NON_PIXEL_SHADER_RESOURCE | 
			D3D12_RESOURCE_USAGE_PIXEL_SHADER_RESOURCE;
	}
	if( ( BindFlags & RsResourceBindFlags::STREAM_OUTPUT ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_STREAM_OUT;
	}
	if( ( BindFlags & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_RENDER_TARGET;
	}
	if( ( BindFlags & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_DEPTH;
	}
	if( ( BindFlags & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_UNORDERED_ACCESS;
	}
	if( ( BindFlags & RsResourceBindFlags::PRESENT ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_PRESENT;
	}

	return static_cast< D3D12_RESOURCE_USAGE >( AllowedUsage );
}