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
RsGeometryBindingDesc& RsGeometryBindingDesc::setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	VertexDeclaration_ = VertexDeclaration;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// setVertexBuffer
RsGeometryBindingDesc& RsGeometryBindingDesc::setVertexBuffer( BcU32 Idx, class RsBuffer* Buffer, BcU32 Stride, BcU32 Offset )
{
	VertexBuffers_[ Idx ].Buffer_ = Buffer;
	VertexBuffers_[ Idx ].Stride_ = Stride;
	VertexBuffers_[ Idx ].Offset_ = Offset;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// setIndexBuffer
RsGeometryBindingDesc& RsGeometryBindingDesc::setIndexBuffer( class RsBuffer* Buffer, BcU32 BytesPerIndex, BcU32 Offset )
{
	BcAssert( ( BytesPerIndex == 2 || BytesPerIndex == 4 ) || ( Buffer == nullptr && BytesPerIndex == 0 ) );
	IndexBuffer_.Buffer_ = Buffer;
	IndexBuffer_.Stride_ = BytesPerIndex;
	IndexBuffer_.Offset_ = Offset;
	return *this;
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
