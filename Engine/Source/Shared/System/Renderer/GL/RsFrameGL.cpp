/**************************************************************************
*
* File:		RsFrameGL.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		GL Frame implementation.
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/GL/RsFrameGL.h"

#include "System/Renderer/GL/RsCoreImplGL.h"

#include "Base/BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// Vertex structures.
class RsPrimitiveNode: public RsRenderNode
{
public:
	void render()
	{
		BcBreakpoint;	
	}
};


//
class RsMaterialPrimitiveNode: public RsPrimitiveNode
{
public:
	void render()
	{
		BcBreakpoint;
	}
};

//
class RsViewportNode: public RsRenderNode
{
public:
	void render()
	{
		pContext_->setViewport( Viewport_ );
	}
	
	RsViewport Viewport_;
};

//////////////////////////////////////////////////////////////////////////
// Ctor
RsFrameGL::RsFrameGL( RsContext* pContext, BcU32 NoofNodes, BcU32 NodeMem )
{
	pContext_ = static_cast< RsContextGL* >( pContext );

	//
	NoofNodes_ = NoofNodes;
	ppNodeArray_ = new RsRenderNode*[ NoofNodes_ ];
	ppNodeSortArray_ = new RsRenderNode*[ NoofNodes_ ];
	CurrNode_ = 0;

	//
	FrameBytes_ = NodeMem;
	pFrameMem_ = new BcU8[ FrameBytes_ ];
	pCurrFrameMem_ = pFrameMem_;


	// Primitive batching.
	pCurrPrimitive_ = NULL;

	//
	reset();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsFrameGL::~RsFrameGL()
{
	//
	pCurrFrameMem_ = NULL;
	delete [] pFrameMem_;
	pFrameMem_ = NULL;

	//
	CurrNode_ = 0;
	delete [] ppNodeArray_;
	delete [] ppNodeSortArray_;
	ppNodeArray_ = NULL;
	ppNodeSortArray_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// getContext
RsContext* RsFrameGL::getContext() const
{
	return pContext_;
}

//////////////////////////////////////////////////////////////////////////
// reset
void RsFrameGL::reset()
{
	// Reset node to 0.
	CurrNode_ = 0;

	// Reset frame memory.
	pCurrFrameMem_ = pFrameMem_;
	pCurrPrimitive_ = NULL;

	// Reset shared nodes.
	pCurrViewport_ = NULL;

	CurrViewport_ = BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// render
void RsFrameGL::render()
{
	// 
	BcAssertMsg( pCurrPrimitive_ == NULL, "RsFrameGL: Unterminated primitive." );

	// Only render if we have a valid context.
	if( pContext_ != NULL )
	{
		// Set default state.
		pContext_->setDefaultState();

		// Sort all nodes.
		sortNodes();

		// Render all nodes.
		for( BcU32 i = 0; i < CurrNode_; ++i )
		{
			RsRenderNode* pRenderNode = ppNodeArray_[ i ];
			pRenderNode->render();

			RsGLCatchError();

			pRenderNode->~RsRenderNode();
		}

		// Reset everything.
		reset();

		// Flip context buffers.
		pContext_->swapBuffers();

		// Catch error.
		RsGLCatchError();
	}
}

//////////////////////////////////////////////////////////////////////////
// setViewport
void RsFrameGL::setViewport( const RsViewport& Viewport )
{
	// Put viewport into the list.
	RsViewportNode* pNode = newObject< RsViewportNode >();
	ppNodeArray_[ CurrNode_++ ] = pNode;
	
	// Advance viewport.
	pCurrViewport_ = pNode;
	CurrViewport_++;

	pNode->Viewport_ = Viewport;
	
	// Assertions.
	BcAssertMsg( CurrViewport_ <= RS_SORT_VIEWPORT_MAX, "RsFrameGL: Viewport limit exceeded." );
	
	// Set node context.
	pNode->pContext_ = pContext_;

	// Set the sort value for the node.
	pNode->Sort_.Value_ = RS_SORT_MACRO_VIEWPORT_RENDERTARGET( CurrViewport_, 0 );
}

//////////////////////////////////////////////////////////////////////////
// addRenderNode
void RsFrameGL::addRenderNode( RsRenderNode* pNode )
{
	BcAssertMsg( pCurrViewport_ != NULL, "RsFrame: No viewport set." );
	
	ppNodeArray_[ CurrNode_++ ] = pNode;
	
	// Set the context for the node.
	pNode->pContext_ = pContext_;

	// Set the sort value for the node.
	pNode->Sort_.Value_ |= RS_SORT_MACRO_VIEWPORT_RENDERTARGET( CurrViewport_, 0 );
}

//////////////////////////////////////////////////////////////////////////
// allocMem
void* RsFrameGL::allocMem( BcSize Bytes )
{
	BcU8* pMem = pCurrFrameMem_;
	pCurrFrameMem_ += Bytes;
	BcAssertMsg( BcU32( pCurrFrameMem_ - pMem ) < FrameBytes_, "RsFrameGL: Out of memory." );
	return pMem;
}

//////////////////////////////////////////////////////////////////////////
// sortNodes
void RsFrameGL::sortNodes()
{
	sortNodeRadix( 0, CurrNode_, ppNodeArray_, ppNodeSortArray_ );
	sortNodeRadix( 8, CurrNode_, ppNodeSortArray_, ppNodeArray_ );
	sortNodeRadix( 16, CurrNode_, ppNodeArray_, ppNodeSortArray_ );
	sortNodeRadix( 24, CurrNode_, ppNodeSortArray_, ppNodeArray_ );
	sortNodeRadix( 32, CurrNode_, ppNodeArray_, ppNodeSortArray_ );
	sortNodeRadix( 40, CurrNode_, ppNodeSortArray_, ppNodeArray_ );
	sortNodeRadix( 48, CurrNode_, ppNodeArray_, ppNodeSortArray_ );
	sortNodeRadix( 56, CurrNode_, ppNodeSortArray_, ppNodeArray_ );
}

//////////////////////////////////////////////////////////////////////////
// sortNodeRadix
void RsFrameGL::sortNodeRadix( BcU32 Bits, BcU32 NoofItems, RsRenderNode** pSrc, RsRenderNode** pDst )
{
	// Basis for implementation by Alice Blunt.
	BcU32 Counts[ 256 ];
	BcU32 Offsets[ 256 ];

	// Initialise counts
	BcMemZero( Counts, sizeof( Counts ) );

	// Count radix frequency
	for ( BcU32 t = 0; t < NoofItems; ++t )
	{
		++Counts[ ( pSrc[ t ]->Sort_.Value_ >> Bits ) & 0xff ];
	}

	// Build offset table
	Offsets[ 0 ] = 0;
	for ( BcU32 t = 1; t < 256; ++t )
	{
		Offsets[ t ] = Offsets[ t - 1 ] + Counts[ t - 1 ];
	}

	// Make sorted array
	for ( BcU32 t = 0; t < NoofItems; ++t )
	{
		pDst[ Offsets[ ( pSrc[ t ]->Sort_.Value_ >> Bits ) & 0xff ]++ ] = pSrc[ t ];
	}
}
