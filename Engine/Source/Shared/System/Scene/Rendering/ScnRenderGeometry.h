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
	DECLARE_RESOURCE( ScnRenderGeometry, CsResource );

public:
	void initialise( 
		const RsVertexDeclarationDesc& VertexDeclarationDesc,
		const RsBufferDesc& IndexBufferDesc,
		const RsBufferDesc& VertexBufferDesc );

	virtual void destroy();


	/**
	 * Set geometry on context.
	 */
	void setOnContext( class RsContext* Context );


private:
	class RsBuffer* IndexBuffer_;
	std::vector< class RsBuffer* > VertexBuffers_;
	class RsVertexDeclaration* VertexDeclaration_;
};

#endif
