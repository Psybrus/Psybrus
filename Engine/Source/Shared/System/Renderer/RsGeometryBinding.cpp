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
void RsGeometryBindingDesc::setVertexBuffer( BcU32 Idx, class RsBuffer* Buffer, BcU32 Stride, BcU32 Offset )
{
	VertexBuffers_[ Idx ].Buffer_ = Buffer;
	VertexBuffers_[ Idx ].Stride_ = Stride;
	VertexBuffers_[ Idx ].Offset_ = Offset;
}

//////////////////////////////////////////////////////////////////////////
// setIndexBuffer
void RsGeometryBindingDesc::setIndexBuffer( class RsBuffer* Buffer, BcU32 BytesPerIndex, BcU32 Offset )
{
	IndexBuffer_.Buffer_ = Buffer;
	IndexBuffer_.Stride_ = BytesPerIndex;
	IndexBuffer_.Offset_ = Offset;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsGeometryBinding::RsGeometryBinding( RsContext* pContext, const RsGeometryBindingDesc & Desc ):
	RsResource( RsResourceType::GEOMETRY_BINDING, pContext ),
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
