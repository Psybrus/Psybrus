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
	void setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration );

	/**
	 * Set vertex buffer to use.
	 */
	void setVertexBuffer( BcU32 Idx, class RsBuffer* Buffer, BcU32 Stride );

	/**
	 * Set index buffer to use.
	 */
	void setIndexBuffer( class RsBuffer* Buffer );


	class RsVertexDeclaration* VertexDeclaration_ = nullptr;

	struct VertexBufferBinding
	{
		RsBuffer* Buffer_ = nullptr;
		BcU32 Stride_ = 0;
	};

	std::array< VertexBufferBinding, 16 > VertexBuffers_ = {};
	class RsBuffer* IndexBuffer_ = nullptr;
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

