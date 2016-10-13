#include "System/Scene/Rendering/ScnRenderMeshImport.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnRenderMeshImport )
	
void ScnRenderMeshImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "ToplogyType_", &ScnRenderMeshImport::ToplogyType_, bcRFF_IMPORTER ),
		new ReField( "NoofIndices_", &ScnRenderMeshImport::NoofIndices_, bcRFF_IMPORTER ),
		new ReField( "NoofVertices_", &ScnRenderMeshImport::NoofVertices_, bcRFF_IMPORTER ),
		new ReField( "VertexStride_", &ScnRenderMeshImport::VertexStride_, bcRFF_IMPORTER ),
		new ReField( "IndexStride_", &ScnRenderMeshImport::IndexStride_, bcRFF_IMPORTER ),
		new ReField( "VertexElements_", &ScnRenderMeshImport::VertexElements_, bcRFF_IMPORTER ),
	};
	
	ReRegisterClass< ScnRenderMeshImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnRenderMeshImport::ScnRenderMeshImport():
	CsResourceImporter( "<INVALID>", "ScnRenderMesh" )
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnRenderMeshImport::ScnRenderMeshImport( 
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
		BcStream&& IndexData ):
	CsResourceImporter( Name, "ScnRenderMesh" ),
	ToplogyType_( ToplogyType ),
	NoofVertices_( NoofVertices ),
	VertexStride_( VertexStride ),
	NoofIndices_( NoofIndices ),
	IndexStride_( IndexStride ),
	VertexElements_( NoofVertexElements ),
	Draws_( NoofDraws ),
	VertexData_( std::move( VertexData ) ),
	IndexData_( std::move( IndexData ) )
{
	for( size_t Idx = 0; Idx < NoofVertexElements; ++Idx )
	{
		VertexElements_[ Idx ] = VertexElements[ Idx ];
	}
	for( size_t Idx = 0; Idx < NoofDraws; ++Idx )
	{
		Draws_[ Idx ] = Draws[ Idx ];
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ScnRenderMeshImport::~ScnRenderMeshImport()
{
}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnRenderMeshImport::import()
{
	if( VertexData_.dataSize() == 0 )
	{
		CsResourceImporter::addMessage( CsMessageCategory::CRITICAL, "Missing 'vertexdata'." );
		return BcFalse;
	}

	if( VertexElements_.size() == 0 )
	{
		CsResourceImporter::addMessage( CsMessageCategory::CRITICAL, "Missing 'vertexelements'." );
		return BcFalse;
	}

	if( VertexData_.dataSize() != ( NoofVertices_ * VertexStride_ ) )
	{
		CsResourceImporter::addMessage( CsMessageCategory::CRITICAL, "'vertexdata' size is invalid." );
		return BcFalse;
	}

	if( IndexData_.dataSize() != ( NoofIndices_ * IndexStride_ ) )
	{
		CsResourceImporter::addMessage( CsMessageCategory::CRITICAL, "'indexdata' size is invalid." );
		return BcFalse;
	}

	ScnRenderMeshFileData Header;
	Header.ToplogyType_ = ToplogyType_;
	Header.NoofVertices_ = NoofVertices_;
	Header.VertexStride_ = VertexStride_;
	Header.NoofIndices_ = NoofIndices_;
	Header.IndexStride_ = IndexStride_;
	Header.NoofVertexElements_ = (BcU32)VertexElements_.size();
	Header.NoofDraws_ = (BcU32)Draws_.size();

	CsResourceImporter::addChunk( BcHash( "header" ), &Header, sizeof( Header ), 16, csPCF_IN_PLACE );
	CsResourceImporter::addChunk( BcHash( "vertexdata" ), VertexData_.pData(), VertexData_.dataSize(), 16, csPCF_COMPRESSED );
	if( Header.NoofIndices_ > 0 )
	{
		CsResourceImporter::addChunk( BcHash( "indexdata" ), IndexData_.pData(), IndexData_.dataSize(), 16, csPCF_COMPRESSED );
	}
	CsResourceImporter::addChunk( BcHash( "vertexelements" ), VertexElements_.data(), VertexElements_.size() * sizeof( RsVertexElement ), 16, csPCF_COMPRESSED );
	CsResourceImporter::addChunk( BcHash( "draws" ), Draws_.data(), Draws_.size() * sizeof( ScnRenderMeshDraw ), 16, csPCF_COMPRESSED );
	return BcTrue;
}

