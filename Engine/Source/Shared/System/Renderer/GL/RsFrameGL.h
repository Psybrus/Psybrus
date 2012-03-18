/**************************************************************************
*
* File:		RsFrameGL.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		GL Frame implementation.
*		
*
*
* 
**************************************************************************/

#ifndef __RsFrameGL_H__
#define __RsFrameGL_H__

#include "System/Renderer/RsFrame.h"
#include "System/Renderer/GL/RsContextGL.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsPrimitiveNode;
class RsRenderTargetNode;
class RsViewportNode;

//////////////////////////////////////////////////////////////////////////
// RsFrameGL
class RsFrameGL:
	public RsFrame
{
public:
	RsFrameGL( RsContext* pContext, BcU32 NoofNodes = 8 * 1024, BcU32 NodeMem = 1024 * 1024 );
	virtual ~RsFrameGL();

	RsContext*		getContext() const;
	void			reset();
	void			render();
	void			setRenderTarget( RsRenderTarget* pRenderTarget );
	void			setViewport( const RsViewport& Viewport );
	void			addRenderNode( RsRenderNode* pInstance );
	
	void			beginPrimitive( eRsPrimitiveType Type, eRsFramePrimitiveMode PrimitiveMode, BcU32 Layer );
	void			endPrimitive();
	void			addLine( const BcVec2d& PointA, const BcVec2d& PointB, const RsColour& Colour, BcU32 Layer );
	void			addLine( const BcVec3d& PointA, const BcVec3d& PointB, const RsColour& Colour, BcU32 Layer );
	void			addBox( const BcVec2d& CornerA, const BcVec2d& CornerB, const RsColour& Colour, BcU32 Layer );

	void			addSprite( RsMaterial* pMaterial, const BcVec2d& Position );

	void			addPrimitive( RsMaterial* pMaterial, RsEffect* pEffect, eRsPrimitiveType Type, BcU32 NoofPrimitives, BcU32 VertexFormat, const void* pVertices, BcU32 Layer, BcU32 Depth );

	void*			allocMem( BcSize Bytes );

private:
	void			sortNodes();
	void			sortNodeRadix( BcU32 Bits, BcU32 NoofItems, RsRenderNode** pSrc, RsRenderNode** pDst );

private:
	struct RsFrameRenderTarget;
	struct RsFrameViewport;

	RsContextGL*			pContext_;

	RsRenderNode**			ppNodeArray_;
	RsRenderNode**			ppNodeSortArray_;
	BcU32					NoofNodes_;
	BcU32					CurrNode_;

	RsPrimitiveNode*		pCurrPrimitive_;

	RsViewportNode*			pCurrViewport_;
	RsRenderTargetNode*		pCurrRenderTarget_;

	BcU32					CurrViewport_;
	BcU32					CurrRenderTarget_;

private:
	BcU8*					pFrameMem_;
	BcU8*					pCurrFrameMem_;
	BcU32					FrameBytes_;
};

#endif
