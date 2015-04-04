#include "System/Renderer/D3D12/RsProgramD3D12.h"
#include "System/Renderer/RsProgram.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramD3D12::RsProgramD3D12( class RsProgram* Parent, ID3D12Device* Device ):
	Parent_( Parent ),
	Device_( Device )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsProgramD3D12::~RsProgramD3D12()
{
}

