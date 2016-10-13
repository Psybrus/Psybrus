#pragma once

#include "System/Content/CsResourceImporter.h"
#include "System/Scene/Rendering/ScnRenderMeshFileData.h"

#include "System/Renderer/RsVertexDeclaration.h"

#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// ScnRenderMeshImport
class ScnRenderMeshImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED( ScnRenderMeshImport, CsResourceImporter );

	ScnRenderMeshImport();
	ScnRenderMeshImport( 
		const std::string Name,
		RsTopologyType ToplogyType,
		BcU32 NoofVertices,
		BcU32 VertexStride,
		BcU32 NoofIndices,
		BcU32 IndexStride,
		size_t NoofVertexElements,
		RsVertexElement* VertexElements,
		size_t NoofDraws,
		ScnRenderMeshDraw* Draws,
		BcStream&& VertexData,
		BcStream&& IndexData );
	virtual ~ScnRenderMeshImport();

	BcBool import() override;

private:
	RsTopologyType ToplogyType_ = RsTopologyType::TRIANGLE_LIST;
	BcU32 NoofVertices_ = 0;
	BcU32 VertexStride_ = 0;
	BcU32 NoofIndices_ = 0;
	BcU32 IndexStride_ = 0;
	std::vector< RsVertexElement > VertexElements_;
	std::vector< ScnRenderMeshDraw > Draws_;

	BcStream VertexData_;
	BcStream IndexData_;
};