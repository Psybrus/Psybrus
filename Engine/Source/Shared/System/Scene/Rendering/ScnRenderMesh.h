#pragma once

#include "Base/BcBinaryData.h"
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

	/**
	 * Construct a render mesh.
	 */
	ScnRenderMesh( ScnRenderMeshFileData Header,
		BcBinaryData VertexData, BcBinaryData IndexData, 
		std::unique_ptr< struct RsVertexElement[] > VertexElements, 
		std::unique_ptr< ScnRenderMeshDraw[] > Draws );
	virtual ~ScnRenderMesh();

	ScnRenderMeshFileData getHeader() const { return Header_; }
	ScnRenderMeshDraw getDraw( size_t Idx ) const { BcAssert( Idx < Header_.NoofDraws_ ); return Draws_[ Idx ]; }
	class RsGeometryBinding* getGeometryBinding() const { return GeometryBinding_.get(); }

private:
	void create() override;
	void destroy() override;

	void fileReady() override;
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData ) override;

private:
	ScnRenderMeshFileData Header_;
	BcBinaryData VertexData_;
	BcBinaryData IndexData_;
	std::unique_ptr< struct RsVertexElement[] > VertexElements_;
	std::unique_ptr< ScnRenderMeshDraw[] > Draws_;

	RsBufferUPtr VertexBuffer_;
	RsBufferUPtr IndexBuffer_;
	RsVertexDeclarationUPtr VertexDeclaration_;
	RsGeometryBindingUPtr GeometryBinding_;
};
