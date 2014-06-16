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
	RsFrameGL( RsContext* pContext, BcU32 NoofNodes = 32 * 1024, BcU32 NodeMem = 1024 * 1024 );
	virtual ~RsFrameGL();

	RsContext*		getContext() const;
	void			reset();
	void			render();
	void			setRenderTarget( RsRenderTarget* pRenderTarget );
	void			setViewport( const RsViewport& Viewport );
	void			addRenderNode( RsRenderNode* pInstance );
	
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
