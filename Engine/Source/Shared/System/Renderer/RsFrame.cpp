/**************************************************************************
*
* File:		RsFrame.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Frame object. All renderable instances are pushed onto here.
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsFrame.h"
#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsContext.h"
#include "System/Renderer/RsRenderNode.h"
#include "System/Os/OsClient.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsFrame::RsFrame( RsContext* pContext, bool ShouldPresent, BcU32 NoofNodes, BcSize NodeMem )
{
	pContext_ = pContext;
	ShouldPresent_ = ShouldPresent;

	//
	NoofNodes_ = NoofNodes;
	ppNodeArray_ = new RsRenderNode*[ NoofNodes_ ];
	ppNodeSortArray_ = new RsRenderNode*[ NoofNodes_ ];
	CurrNode_ = 0;

	//
	FrameBytes_ = NodeMem;
	pFrameMem_ = new BcU8[ FrameBytes_ ];
	pCurrFrameMem_ = pFrameMem_;

	//
	reset();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsFrame::~RsFrame()
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
RsContext* RsFrame::getContext() const
{
	return pContext_;
}

//////////////////////////////////////////////////////////////////////////
// getBackBufferWidth
BcU32 RsFrame::getBackBufferWidth() const
{
	return Width_;
}
	
//////////////////////////////////////////////////////////////////////////
// getBackBufferHeight
BcU32 RsFrame::getBackBufferHeight() const
{
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// reset
void RsFrame::reset()
{
	// Reset node to 0.
	CurrNode_ = 0;

	// Reset frame memory.
	pCurrFrameMem_ = pFrameMem_;

	// Cache width + height from client.
	Width_ = pContext_->getClient()->getWidth();
	Height_ = pContext_->getClient()->getHeight();
}

//////////////////////////////////////////////////////////////////////////
// render
void RsFrame::render()
{
	PSY_PROFILER_SECTION( "RsFrame::render" );
	PSY_PROFILER_GPU_SECTION( "RsFrame::render" );

	// Only render if we have a valid context.
	if( pContext_ != NULL )
	{
		// Resize backbuffer if we should present.
		if( ShouldPresent_ )
		{
			pContext_->resizeBackBuffer( Width_, Height_ );
		}

		// Begin frame.
		pContext_->beginFrame();

		// Sort all nodes.
		sortNodes();

		// Render all nodes.
		for( BcU32 i = 0; i < CurrNode_; ++i )
		{
			RsRenderNode* pRenderNode = ppNodeArray_[ i ];
			pRenderNode->render( pContext_ );
		}

		// End frame.
		pContext_->endFrame();

		// Present if enabled.
		if( ShouldPresent_ )
		{
			pContext_->present();
		}

		// Reset everything.
		reset();
	}
}

//////////////////////////////////////////////////////////////////////////
// allocMem
void* RsFrame::allocMem( BcSize Bytes )
{
	Bytes = BcCalcAlignment( static_cast< BcU64 >( Bytes ), static_cast< BcU64 >( 64 ) );
	BcU8* pMem = pCurrFrameMem_;
	pCurrFrameMem_ += Bytes;
	BcAssertMsg( BcSize( pCurrFrameMem_ - pFrameMem_ ) < FrameBytes_, "RsFrame: Out of memory." );
	return pMem;
}

//////////////////////////////////////////////////////////////////////////
// sortNodes
void RsFrame::sortNodes()
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
void RsFrame::sortNodeRadix( BcU32 Bits, BcU32 NoofItems, RsRenderNode** pSrc, RsRenderNode** pDst )
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
