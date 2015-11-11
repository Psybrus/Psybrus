#include "System/Renderer/RsGeometryBinding.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsVertexDeclaration.h"

//////////////////////////////////////////////////////////////////////////
// RsGeometryBindingDesc
RsGeometryBindingDesc::RsGeometryBindingDesc()
{
}

//////////////////////////////////////////////////////////////////////////
// setVertexDeclaration
void RsGeometryBindingDesc::setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	VertexDeclaration_ = VertexDeclaration;
}

//////////////////////////////////////////////////////////////////////////
// setVertexBuffer
void RsGeometryBindingDesc::setVertexBuffer( BcU32 Idx, class RsBuffer* Buffer, BcU32 Stride )
{
	VertexBuffers_[ Idx ].Buffer_ = Buffer;
	VertexBuffers_[ Idx ].Stride_ = Stride;
}

//////////////////////////////////////////////////////////////////////////
// setIndexBuffer
void RsGeometryBindingDesc::setIndexBuffer( class RsBuffer* Buffer )
{
	IndexBuffer_ = Buffer;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsGeometryBinding::RsGeometryBinding( RsContext* pContext, const RsGeometryBindingDesc & Desc ):
	RsResource( pContext ),
	Desc_( Desc )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsGeometryBinding::~RsGeometryBinding()
{
}

//////////////////////////////////////////////////////////////////////////
// getDesc
const RsGeometryBindingDesc& RsGeometryBinding::getDesc() const
{
	return Desc_;
}
