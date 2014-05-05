/**************************************************************************
*
* File:		RsIndexBufferD3D11.cpp
* Author:	Neil Richardson
* Ver/Date:	25/02/11
* Description:
*
*
*
*
*
**************************************************************************/

#include "System/Renderer/D3D11/RsIndexBufferD3D11.h"
#include "System/Renderer/D3D11/RsContextD3D11.h"
#include "System/Renderer/RsCore.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsIndexBufferD3D11::RsIndexBufferD3D11( RsContext* pContext, const RsIndexBufferDesc& Desc, void* pIndexData ):
	RsIndexBuffer( pContext )
{
	// Setup stride and descriptor.
	Desc_ = Desc;
	pData_ = pIndexData;
	DataSize_ = Desc.NoofIndices_ * sizeof( BcU16 );

	// Create data if we need to.
	if( pData_ == NULL )
	{
		pData_ = new BcU8[ DataSize_ ];
		DeleteData_ = BcTrue;
	}

	// Buffer desc.
	BufferDesc_.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc_.ByteWidth = DataSize_;
	BufferDesc_.BindFlags = D3D11_BIND_INDEX_BUFFER;
	BufferDesc_.CPUAccessFlags = 0;
	BufferDesc_.MiscFlags = 0;

	// Resource data.
	ResourceData_.pSysMem = pData_;
	ResourceData_.SysMemPitch = 0;
	ResourceData_.SysMemSlicePitch = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsIndexBufferD3D11::~RsIndexBufferD3D11()
{

}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsIndexBufferD3D11::create()
{
	RsContextD3D11* Context = static_cast< RsContextD3D11* >( RsCore::pImpl()->getContext( nullptr ) );

	// Generate buffers.
	ID3D11Buffer* Buffer = nullptr; 
	HRESULT Result = Context->device()->CreateBuffer( &BufferDesc_, &ResourceData_, &Buffer );
	if( Result == S_OK )
	{
		setHandle( Buffer );
		UpdateSyncFence_.increment();
		update();
	}
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsIndexBufferD3D11::update()
{
	RsContextD3D11* Context = static_cast< RsContextD3D11* >( RsCore::pImpl()->getContext( nullptr ) );
	ID3D11Buffer* Buffer = getHandle< ID3D11Buffer* >();
	if( pData_ != NULL )
	{
		Context->deviceContext()->Map( Buffer );

		UpdateSyncFence_.decrement();
	}
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsIndexBufferD3D11::destroy()
{
	GLuint Handle = getHandle< GLuint >();

	if( Handle != 0 )
	{
		glDeleteBuffers( 1, &Handle );
		setHandle< GLuint >( 0 );
	}
}

////////////////////////////////////////////////////////////////////////////////
// bind
void RsIndexBufferD3D11::bind()
{

}
