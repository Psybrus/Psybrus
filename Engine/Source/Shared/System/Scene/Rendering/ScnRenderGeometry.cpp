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
		ReField( "Primitive_",				&ScnRenderGeometry::Primitive_ ),
	};
	
	ReRegisterClass< ScnRenderGeometry, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnRenderGeometry::initialise(
	const RsVertexDeclarationDesc& VertexDeclarationDesc,
	const RsIndexBufferDesc& IndexBufferDesc,
	const RsVertexBufferDesc& VertexBufferDesc )
{
	Super::initialise();

	IndexBuffer_ = RsCore::pImpl()->createIndexBuffer( IndexBufferDesc );
	VertexBuffers_.push_back( RsCore::pImpl()->createVertexBuffer( VertexBufferDesc ) );
	VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration( VertexDeclarationDesc );

	RsPrimitiveDesc PrimitiveDesc = RsPrimitiveDesc( VertexDeclaration_ )
		.setIndexBuffer( IndexBuffer_ )
		.setVertexBuffer( 0, VertexBuffers_[ 0 ] );
	Primitive_ = RsCore::pImpl()->createPrimitive( PrimitiveDesc );
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual 
void ScnRenderGeometry::destroy()
{
	Super::destroy();

	RsCore::pImpl()->destroyResource( Primitive_ );
	RsCore::pImpl()->destroyResource( VertexDeclaration_ );
	for( auto VertexBuffer : VertexBuffers_ )
	{
		RsCore::pImpl()->destroyResource( VertexBuffer );
	}
	RsCore::pImpl()->destroyResource( IndexBuffer_ );
}
