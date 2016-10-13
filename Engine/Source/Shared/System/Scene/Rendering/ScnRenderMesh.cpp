#include "System/Scene/Rendering/ScnRenderMesh.h"

#include "System/Renderer/RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnRenderMesh );

void ScnRenderMesh::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Header_", &ScnRenderMesh::Header_, bcRFF_CHUNK_DATA ),
	};
	
	auto& Class = ReRegisterClass< ScnRenderMesh, Super >( Fields );
	BcUnusedVar( Class );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnRenderMesh::ScnRenderMesh()
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnRenderMesh::ScnRenderMesh( ScnRenderMeshFileData Header,
		BcBinaryData VertexData, BcBinaryData IndexData, 
		std::unique_ptr< struct RsVertexElement[] > VertexElements, 
		std::unique_ptr< ScnRenderMeshDraw[] > Draws ):
	Header_( Header ),
	VertexData_( std::move( VertexData ) ),
	IndexData_( std::move( IndexData ) ),
	VertexElements_( std::move( VertexElements ) ),
	Draws_( std::move( Draws ) )
{
	BcAssert( VertexElements_ != nullptr );
	BcAssert( Draws_ != nullptr );
	markCreate();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ScnRenderMesh::~ScnRenderMesh()
{
}

//////////////////////////////////////////////////////////////////////////
// create
void ScnRenderMesh::create()
{
#if !PSY_PRODUCTION
	const std::string DebugName = getFullName();
	const char* DebugNameCStr = DebugName.c_str();
#else
	const char* DebugNameCStr = nullptr;
#endif

	// Fence for 2 uploads.
	SysFence UpdateFence( 2 );

	VertexBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc( RsResourceBindFlags::VERTEX_BUFFER, RsResourceCreationFlags::STATIC,
			Header_.NoofVertices_ * Header_.VertexStride_, 0 ), DebugNameCStr );

	RsCore::pImpl()->updateBuffer( VertexBuffer_.get(), 0, Header_.NoofVertices_ * Header_.VertexStride_, 
		RsResourceUpdateFlags::ASYNC, 
		[ this, &UpdateFence ]( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			memcpy( Lock.Buffer_, VertexData_.getData< BcU8 >(), Buffer->getDesc().SizeBytes_ );
			UpdateFence.decrement();
		} );

	if( Header_.NoofIndices_ > 0 )
	{
		IndexBuffer_ = RsCore::pImpl()->createBuffer(
			RsBufferDesc( RsResourceBindFlags::INDEX_BUFFER, RsResourceCreationFlags::STATIC,
				Header_.NoofIndices_ * Header_.IndexStride_, 0 ), DebugNameCStr );

		RsCore::pImpl()->updateBuffer( IndexBuffer_.get(), 0, Header_.NoofIndices_ * Header_.IndexStride_, 
			RsResourceUpdateFlags::ASYNC, 
			[ this, &UpdateFence ]( RsBuffer* Buffer, const RsBufferLock& Lock )
			{
				memcpy( Lock.Buffer_, IndexData_.getData< BcU8 >(), Buffer->getDesc().SizeBytes_ );
				UpdateFence.decrement();
			} );
	}

	VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration(
		RsVertexDeclarationDesc( VertexElements_.get(), Header_.NoofVertexElements_ ), DebugNameCStr );

	GeometryBinding_ = RsCore::pImpl()->createGeometryBinding( 
		RsGeometryBindingDesc()
			.setVertexBuffer( 0, VertexBuffer_.get(), Header_.VertexStride_, 0 )
			.setIndexBuffer( IndexBuffer_.get(), Header_.IndexStride_, 0 )
			.setVertexDeclaration( VertexDeclaration_.get() ), DebugNameCStr );

	UpdateFence.wait();

	// Free data now that we're done with it.
	VertexData_ = BcBinaryData();
	IndexData_ = BcBinaryData();
	VertexElements_.reset();
	VertexElements_ = nullptr;

	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
void ScnRenderMesh::destroy()
{
	GeometryBinding_.reset();
	VertexDeclaration_.reset();
	IndexBuffer_.reset();
	VertexBuffer_.reset();
	Draws_.reset();
}

//////////////////////////////////////////////////////////////////////////
// flleReady
void ScnRenderMesh::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0, &Header_ );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnRenderMesh::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "header" ) )
	{
		// Allocate all data.
		VertexData_ = BcBinaryData( Header_.NoofVertices_ * Header_.VertexStride_ );
		IndexData_ = BcBinaryData( Header_.NoofIndices_ * Header_.IndexStride_ );
		VertexElements_.reset( new RsVertexElement[ Header_.NoofVertexElements_ ] );
		Draws_.reset( new ScnRenderMeshDraw[ Header_.NoofDraws_ ] );
		BcU32 Idx = 1;
		requestChunk( Idx++, VertexData_.getData< BcU8 >() );
		if( Header_.NoofIndices_ > 0 )
		{
			requestChunk( Idx++, IndexData_.getData< BcU8 >() );
		}
		requestChunk( Idx++, VertexElements_.get() );
		requestChunk( Idx++, Draws_.get() );
	}
	else if( ChunkID == BcHash( "vertices" ) )
	{
		BcAssert( VertexData_.getData< BcU8 >() == pData );
	}
	else if( ChunkID == BcHash( "indices" ) )
	{
		BcAssert( IndexData_.getData< BcU8 >() == pData );
	}
	else if( ChunkID == BcHash( "vertexelements" ) )
	{
		BcAssert( VertexElements_.get() == pData );
	}
	else if( ChunkID == BcHash( "draws" ) )
	{
		BcAssert( Draws_.get() == pData );
		markCreate();
	}
}
