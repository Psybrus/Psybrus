/**************************************************************************
 *
 * File:	RsIndexBufferD3D11.h
 * Author: 	Neil Richardson 
 * Ver/Date:	
 * Description:
 *		D3D11 index buffer.
 *		
 *
 *
 * 
 **************************************************************************/

#ifndef __RSINDEXBUFFERD3D11_H__
#define __RSINDEXBUFFERD3D11_H__

#include "System/Renderer/RsIndexBuffer.h"
#include "System/Renderer/D3D11/RsD3D11.h"

////////////////////////////////////////////////////////////////////////////////
// RsIndexBufferD3D11
class RsIndexBufferD3D11:
	public RsIndexBuffer
{
public:
	/**
	 * Create index buffer.
	 * @param Desc Buffer descriptor.
	 * @param pIndexData Pointer to index data, NULL to create own.
	 */
	RsIndexBufferD3D11( RsContext* pContext, const RsIndexBufferDesc& Desc, void* pIndexData );
	virtual ~RsIndexBufferD3D11();
	
protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	
		
private:
	RsIndexBufferDesc					Desc_;
	D3D11_BUFFER_DESC					BufferDesc_;
	D3D11_SUBRESOURCE_DATA				ResourceData_;
};

#endif
