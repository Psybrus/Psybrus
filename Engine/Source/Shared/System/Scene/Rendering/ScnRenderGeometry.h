/**************************************************************************
*
* File:		ScnRenderGeometry.h
* Author:	Neil Richardson 
* Ver/Date:		
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnRenderGeometry_H__
#define __ScnRenderGeometry_H__

#include "System/Renderer/RsCore.h"
#include "System/Content/CsResource.h"

//////////////////////////////////////////////////////////////////////////
// ScnRenderGeometry
class ScnRenderGeometry:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnRenderGeometry );

public:
	void initialise( 
		const RsVertexDeclarationDesc& VertexDeclarationDesc,
		const RsIndexBufferDesc& IndexBufferDesc,
		const RsVertexBufferDesc& VertexBufferDesc );

	virtual void destroy();



private:
	class RsIndexBuffer* IndexBuffer_;
	std::vector< class RsVertexBuffer* > VertexBuffers_;
	class RsVertexDeclaration* VertexDeclaration_;
};

#endif
