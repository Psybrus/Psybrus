/**************************************************************************
*
* File:		ScnRenderGeometry.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Rendering/ScnRenderGeometry.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnRenderGeometry );

void ScnRenderGeometry::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "IndexBuffer_",			&ScnRenderGeometry::IndexBuffer_ ),
		ReField( "VertexBuffers_",			&ScnRenderGeometry::VertexBuffers_ ),
		ReField( "VertexDeclaration_",		&ScnRenderGeometry::VertexDeclaration_ ),
	};
	
	ReRegisterClass< ScnRenderGeometry, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnRenderGeometry::initialise(
	const RsVertexDeclarationDesc& VertexDeclarationDesc,
	const RsBufferDesc& IndexBufferDesc,
	const RsBufferDesc& VertexBufferDesc )
{
	Super::initialise();

	IndexBuffer_ = RsCore::pImpl()->createBuffer( IndexBufferDesc );
	VertexBuffers_.push_back( RsCore::pImpl()->createBuffer( VertexBufferDesc ) );
	VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration( VertexDeclarationDesc );
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual 
void ScnRenderGeometry::destroy()
{
	Super::destroy();

	RsCore::pImpl()->destroyResource( VertexDeclaration_ );
	for( auto VertexBuffer : VertexBuffers_ )
	{
		RsCore::pImpl()->destroyResource( VertexBuffer );
	}
	RsCore::pImpl()->destroyResource( IndexBuffer_ );
}

//////////////////////////////////////////////////////////////////////////
// setOnContext
void ScnRenderGeometry::setOnContext( class RsContext* Context )
{
	// TODO: Multiple vertex buffers.
	Context->setVertexDeclaration( VertexDeclaration_ );
	Context->setIndexBuffer( IndexBuffer_ );
	//Context->setVertexBuffer( 0, VertexBuffers_[ 0 ] );
}
