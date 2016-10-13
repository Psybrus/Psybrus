#pragma once

#include "System/Content/CsResource.h"
#include "System/Renderer/RsUniquePointers.h"
#include "System/Scene/Rendering/ScnRenderMeshFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnRenderMesh
class ScnRenderMesh : public CsResource
{
public:
	REFLECTION_DECLARE_DERIVED( ScnRenderMesh, CsResource );

	ScnRenderMesh();
	virtual ~ScnRenderMesh();

	ScnRenderMeshFileData getHeader() const { return Header_; }
	ScnRenderMeshDraw getDraw( size_t Idx ) const { BcAssert( Idx < Header_.NoofDraws_ ); return Draws_.get()[ Idx ]; }
	class RsGeometryBinding* getGeometryBinding() const { return GeometryBinding_.get(); }

private:
	void create() override;
	void destroy() override;

	void fileReady() override;
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData ) override;

private:
	ScnRenderMeshFileData Header_;
	void* VertexData_ = nullptr;
	void* IndexData_ = nullptr;
	std::unique_ptr< struct RsVertexElement > VertexElements_;
	std::unique_ptr< ScnRenderMeshDraw > Draws_;

	RsBufferUPtr VertexBuffer_;
	RsBufferUPtr IndexBuffer_;
	RsVertexDeclarationUPtr VertexDeclaration_;
	RsGeometryBindingUPtr GeometryBinding_;
};
