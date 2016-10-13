#pragma once

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsGeometryBindingDesc
struct RsGeometryBindingDesc
{
public:
	RsGeometryBindingDesc();

	/**
	 * Set vertex declaration to use for geometry binding.
	 */
	RsGeometryBindingDesc& setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration );

	/**
	 * Set vertex buffer to use.
	 * @param Idx Stream index.
	 * @param Buffer Buffer to bind.
	 * @param Stride Stride per vertex in buffer.
	 * @param Offset Offset from start of buffer in bytes to bind from.
	 */
	RsGeometryBindingDesc& setVertexBuffer( BcU32 Idx, class RsBuffer* Buffer, BcU32 Stride, BcU32 Offset = 0 );

	/**
	 * Set index buffer to use.
	 * @param Buffer Buffer to bind.
	 * @param BytesPerIndex Bytes per index. 2, or 4 bytes.
	 * @param Offset Offset from start of buffer in bytes to bind from.
	 */
	RsGeometryBindingDesc& setIndexBuffer( class RsBuffer* Buffer, BcU32 BytesPerIndex = 2, BcU32 Offset = 0 );


	class RsVertexDeclaration* VertexDeclaration_ = nullptr;

	struct BufferBinding
	{
		RsBuffer* Buffer_ = nullptr;
		BcU32 Stride_ = 0;
		BcU32 Offset_ = 0;
	};

	std::array< BufferBinding, 16 > VertexBuffers_ = {};
	BufferBinding IndexBuffer_ = BufferBinding();
};

////////////////////////////////////////////////////////////////////////////////
// RsGeometryBinding
class RsGeometryBinding:
	public RsResource
{
public:
	RsGeometryBinding( class RsContext* pContext, const RsGeometryBindingDesc & Desc );
	virtual ~RsGeometryBinding();

	const RsGeometryBindingDesc& getDesc() const;

private:
	RsGeometryBindingDesc Desc_;
};

