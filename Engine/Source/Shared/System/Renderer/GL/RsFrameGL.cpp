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

#include "RsFrameGL.h"

#include "RsCoreImplGL.h"
#include "RsRenderTargetGL.h"

#include "BcMemory.h"

// NEILO HACK.
extern BcU32 GResolutionWidth;
extern BcU32 GResolutionHeight;

//////////////////////////////////////////////////////////////////////////
// Vertex structures.
struct TVertex2D
{
	BcF32 X_, Y_;
	BcF32 U_, V_;
	BcU32 Colour_;

	static const BcU32 VERTEX_TYPE = rsVDF_POSITION_XY | rsVDF_TEXCOORD_UV0 | rsVDF_COLOUR_RGBA8;
};

struct TVertex3D
{
	BcF32 X_, Y_, Z_;
	BcF32 U_, V_;
	BcU32 Colour_;
	
	static const BcU32 VERTEX_TYPE = rsVDF_POSITION_XYZ | rsVDF_TEXCOORD_UV0 | rsVDF_COLOUR_RGBA8;
};

//
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
		//RsCore::pImpl< RsCoreImplGL >()->setViewport( &Viewport_ );	
	}

	RsViewport Viewport_;
};

//
class RsRenderTargetNode: public RsRenderNode
{
public:
	void render()
	{
 		if( pRenderTarget_ != NULL )
		{
			GLuint Handle = pRenderTarget_->getHandle< GLuint >();
			glBindFramebuffer( GL_FRAMEBUFFER, Handle );
			
			glViewport( 0, 0, pRenderTarget_->width(), pRenderTarget_->height() );
		}
		else
		{
			glBindFramebuffer( GL_FRAMEBUFFER, 0 );
			glViewport( 0, 0, GResolutionWidth, GResolutionHeight );
		}

		
		// Enable depth write to clear screen.
		RsStateBlock* pStateBlock = RsCore::pImpl()->getStateBlock();
		pStateBlock->bind();
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );		
	}
	
	RsRenderTarget* pRenderTarget_;
};


//////////////////////////////////////////////////////////////////////////
// Ctor
RsFrameGL::RsFrameGL( BcHandle DeviceHandle, BcU32 Width, BcU32 Height, BcU32 NoofNodes, BcU32 NodeMem )
{
	DeviceHandle_ = DeviceHandle;
	Width_ = Width;
	Height_ = Height;

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
// deviceHandle
BcHandle RsFrameGL::deviceHandle() const
{
	return DeviceHandle_;
}

//////////////////////////////////////////////////////////////////////////
// width
BcU32 RsFrameGL::width() const
{
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// height
BcU32 RsFrameGL::height() const
{
	return Height_;
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
	pCurrRenderTarget_ = NULL;

	CurrViewport_ = BcErrorCode;
	CurrRenderTarget_ = BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// render
void RsFrameGL::render()
{
	// 
	BcAssertMsg( pCurrPrimitive_ == NULL, "RsFrameGL: Unterminated primitive." );

	// Set default state.
	RsCore::pImpl< RsCoreImplGL >()->getStateBlock()->setDefaultState();

	// Sort all nodes.
	sortNodes();

	// Render all nodes.
	for( BcU32 i = 0; i < CurrNode_; ++i )
	{
		RsRenderNode* pRenderNode = ppNodeArray_[ i ];
		pRenderNode->render();
		pRenderNode->~RsRenderNode();
	}

	// Reset everything.
	reset();

	// Flush and flip.
	glFlush();
	
#if PLATFORM_OSX
	// Flush buffer.
	OsViewOSX_Interface::FlushBuffer();
#elif PLATFORM_WINDOWS
	// Flip.
	extern BcHandle GWindowDC_;
	::SwapBuffers( (HDC)GWindowDC_ );
#endif


	// TEMP HACK: Free frame.
	delete this;
}

//////////////////////////////////////////////////////////////////////////
// setRenderTarget
void RsFrameGL::setRenderTarget( RsRenderTarget* pRenderTarget )
{
	BcUnusedVar( pRenderTarget );

	// Put render target into array.
	RsRenderTargetNode* pNode = newObject< RsRenderTargetNode >();
	ppNodeArray_[ CurrNode_++ ] = pNode;
	
	// Set the current viewport, rendertarget, etc.
	//pCurrViewport_ = NULL; // TODO: FIX.
	//CurrViewport_ = BcErrorCode;
	pCurrRenderTarget_ = pNode;
	CurrRenderTarget_++;
	
	// Set rendertarget for node.
	pNode->pRenderTarget_ = (RsRenderTargetGL*)pRenderTarget;
	
	// Set the sort value for the node.
	pNode->Sort_.Value_ = RS_SORT_MACRO_VIEWPORT_RENDERTARGET( 0, CurrRenderTarget_ );
}

//////////////////////////////////////////////////////////////////////////
// setViewport
void RsFrameGL::setViewport( const RsViewport& Viewport )
{
	// Assertions.
	BcAssertMsg( pCurrRenderTarget_ != NULL, "RsFrameGL: Render target not set." );
	
	// Put viewport into the list.
	RsViewportNode* pNode = newObject< RsViewportNode >();
	ppNodeArray_[ CurrNode_++ ] = pNode;
	
	// Advance viewport.
	pCurrViewport_ = pNode;
	CurrViewport_++;

	pNode->Viewport_ = Viewport;
	
	// Assertions.
	BcAssertMsg( CurrViewport_ <= RS_SORT_VIEWPORT_MAX, "RsFrameGL: Viewport limit exceeded." );
	
	// Set the sort value for the node.
	pNode->Sort_.Value_ = RS_SORT_MACRO_VIEWPORT_RENDERTARGET( CurrViewport_, CurrRenderTarget_ );
}

//////////////////////////////////////////////////////////////////////////
// addInstance
void RsFrameGL::addRenderNode( RsRenderNode* pNode )
{
	BcAssertMsg( pCurrRenderTarget_ != NULL, "RsFrame: No render target set." );
	BcAssertMsg( pCurrViewport_ != NULL, "RsFrame: No viewport set." );
	
	ppNodeArray_[ CurrNode_++ ] = pNode;
	
	// Set the sort value for the node.
	pNode->Sort_.Value_ |= RS_SORT_MACRO_VIEWPORT_RENDERTARGET( CurrViewport_, CurrRenderTarget_ );
}

//////////////////////////////////////////////////////////////////////////
// beginPrimitive
void RsFrameGL::beginPrimitive( eRsPrimitiveType Type, eRsFramePrimitiveMode PrimitiveMode, BcU32 Layer )
{
	BcAssertMsg( pCurrPrimitive_ == NULL, "RsFrameGL: Primitive already started." );
	BcBreakpoint;
	
	BcUnusedVar( Type );
	BcUnusedVar( PrimitiveMode );
	BcUnusedVar( Layer );
	/*
	// Create node for rendering.
	RsPrimitiveNode* pNode = newObject< RsPrimitiveNode >();

	// Setup primitive.
	switch( PrimitiveMode )
	{
	case rsFPM_2D:
		{
			pNode->pEffect_ = RsCore::pImpl()->getDefaultEffect( rsFX_GEO_2D );
			pNode->VertexFormat_ = TVertex2D::VERTEX_TYPE;
			pNode->VertexStride_ = sizeof( TVertex2D );
		}
		break;

	case rsFPM_3D:
		{
			pNode->pEffect_ = RsCore::pImpl()->getDefaultEffect( rsFX_GEO_3D );
			pNode->VertexFormat_ = TVertex3D::VERTEX_TYPE;
			pNode->VertexStride_ = sizeof( TVertex3D );
			break;
		}
	}

	// Setup sort.
	RsRenderSort Sort;

	BcAssert( Layer <= RS_SORT_LAYER_MAX );
	Sort.Value_ = 0;
	Sort.Depth_ = RS_SORT_DEPTH_MAX;
	Sort.Blend_ = rsBM_BLEND;
	Sort.Pass_ = RS_SORT_PASS_FORWARD;
	Sort.Layer_ = Layer;
	pNode->Sort_.Value_ |= Sort.Value_;

	// Basic info.
	pNode->PrimType_= Type;
	pNode->NoofPrims_ = 0;
	pNode->pVertices_ = allocMem( 0 );

	pCurrPrimitive_ = pNode;
	*/
}

//////////////////////////////////////////////////////////////////////////
// endPrimitive
void RsFrameGL::endPrimitive()
{
	BcBreakpoint;

	/*
	addRenderNode( pCurrPrimitive_ );
	pCurrPrimitive_ = NULL;
	*/
}

//////////////////////////////////////////////////////////////////////////
// addLine
void RsFrameGL::addLine( const BcVec2d& PointA, const BcVec2d& PointB, const RsColour& Colour, BcU32 Layer )
{
	BcBreakpoint;

	BcUnusedVar( PointA );
	BcUnusedVar( PointB );
	BcUnusedVar( Colour );
	BcUnusedVar( Layer );

	/*

	// Create instance for rendering.
	RsPrimitiveNode* pNode = pCurrPrimitive_;
	
	// If we're not in the middle of a prim list, alloc a new node.
	if( pNode == NULL )
	{
		pNode = newObject< RsPrimitiveNode >();
	}
	
	// Setup vertices.
	TVertex2D* pVertices = alloc< TVertex2D >( 2 );
	
	BcU32 CachedColour = Colour.asARGB();
	
	pVertices[0].X_ = PointA.x();
	pVertices[0].Y_ = PointA.y();
	pVertices[0].Colour_ = CachedColour;
	
	pVertices[1].X_ = PointB.x();
	pVertices[1].Y_ = PointB.y();
	pVertices[1].Colour_ = CachedColour;
	
	// If we've got no prim list, just add the node now.
	if( pCurrPrimitive_ == NULL )
	{
		pNode->pEffect_ = RsCore::pImpl()->getDefaultEffect( rsFX_GEO_2D );
		pNode->PrimType_= rsPT_LINELIST;
		pNode->NoofPrims_ = 1;
		pNode->VertexFormat_ = TVertex2D::VERTEX_TYPE;
		pNode->VertexStride_ = sizeof( TVertex2D );
		pNode->pVertices_ = pVertices;

		// Setup sort.
		BcAssert( Layer <= RS_SORT_LAYER_MAX );
		RsRenderSort Sort;

		Sort.Value_ = 0;
		Sort.Blend_ = rsBM_BLEND;
		Sort.Pass_ = RS_SORT_PASS_FORWARD;
		Sort.Layer_ = Layer;
		pNode->Sort_.Value_ |= Sort.Value_;

		addRenderNode( pNode );
	}
	else
	{
		BcAssert( pNode->PrimType_ == rsPT_LINELIST );
		pNode->NoofPrims_ += 1;
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
// addLine
void RsFrameGL::addLine( const BcVec3d& PointA, const BcVec3d& PointB, const RsColour& Colour, BcU32 Layer )
{
	BcBreakpoint;

	BcUnusedVar( PointA );
	BcUnusedVar( PointB );
	BcUnusedVar( Colour );
	BcUnusedVar( Layer );

	/*

	// Create instance for rendering.
	RsPrimitiveNode* pNode = pCurrPrimitive_;

	// If we're not in the middle of a prim list, alloc a new node.
	if( pNode == NULL )
	{
		pNode = newObject< RsPrimitiveNode >();
	}

	// Setup vertices.
	TVertex3D* pVertices = alloc< TVertex3D >( 2 );

	BcU32 CachedColour = Colour.asARGB();

	pVertices[0].X_ = PointA.x();
	pVertices[0].Y_ = PointA.y();
	pVertices[0].Z_ = PointA.z();
	pVertices[0].Colour_ = CachedColour;

	pVertices[1].X_ = PointB.x();
	pVertices[1].Y_ = PointB.y();
	pVertices[1].Z_ = PointB.z();
	pVertices[1].Colour_ = CachedColour;

	// If we've got no prim list, just add the node now.
	if( pCurrPrimitive_ == NULL )
	{	
		pNode->pEffect_ = RsCore::pImpl()->getDefaultEffect( rsFX_GEO_3D );
		pNode->PrimType_= rsPT_LINELIST;
		pNode->NoofPrims_ = 1;
		pNode->VertexFormat_ = TVertex3D::VERTEX_TYPE;
		pNode->VertexStride_ = sizeof( TVertex3D );
		pNode->pVertices_ = pVertices;
		
		// Setup sort.
		BcAssert( Layer <= RS_SORT_LAYER_MAX );
		RsRenderSort Sort;

		Sort.Value_ = 0;
		Sort.Depth_ = RS_SORT_DEPTH_MAX;
		Sort.Blend_ = rsBM_BLEND;
		Sort.Pass_ = RS_SORT_PASS_FORWARD;
		Sort.Layer_ = Layer;
		pNode->Sort_.Value_ |= Sort.Value_;

		addRenderNode( pNode );
	}
	else
	{
		BcAssert( pNode->PrimType_ == rsPT_LINELIST );
		pNode->NoofPrims_ += 1;
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
// addBox
void RsFrameGL::addBox( const BcVec2d& CornerA, const BcVec2d& CornerB, const RsColour& Colour, BcU32 Layer )
{
	BcBreakpoint;

	BcUnusedVar( CornerA );
	BcUnusedVar( CornerB );
	BcUnusedVar( Colour );
	BcUnusedVar( Layer );

	/*
	// Create instance for rendering.
	RsPrimitiveNode* pNode = newObject< RsPrimitiveNode >();

	// Setup vertices.
	TVertex2D* pVertices = alloc< TVertex2D >( 4 );

	BcU32 CachedColour = Colour.asARGB();

	pVertices[0].X_ = CornerA.x();
	pVertices[0].Y_ = CornerA.y();
	pVertices[0].Colour_ = CachedColour;

	pVertices[1].X_ = CornerB.x();
	pVertices[1].Y_ = CornerA.y();
	pVertices[1].Colour_ = CachedColour;

	pVertices[2].X_ = CornerA.x();
	pVertices[2].Y_ = CornerB.y();
	pVertices[2].Colour_ = CachedColour;

	pVertices[3].X_ = CornerB.x();
	pVertices[3].Y_ = CornerB.y();
	pVertices[3].Colour_ = CachedColour;

	// If we've got no prim list, just add the node now.
	pNode->pEffect_ = RsCore::pImpl()->getDefaultEffect( rsFX_GEO_2D );
	pNode->PrimType_= rsPT_TRIANGLESTRIP;
	pNode->NoofPrims_ = 2;
	pNode->VertexFormat_ = TVertex2D::VERTEX_TYPE;
	pNode->VertexStride_ = sizeof( TVertex2D );
	pNode->pVertices_ = pVertices;

	// Setup sort.
	BcAssert( Layer <= RS_SORT_LAYER_MAX );
	RsRenderSort Sort;

	Sort.Value_ = 0;
	Sort.Depth_ = RS_SORT_DEPTH_MAX;
	Sort.Blend_ = rsBM_BLEND;
	Sort.Pass_ = RS_SORT_PASS_FORWARD;
	Sort.Layer_ = Layer;
	pNode->Sort_.Value_ |= Sort.Value_;

	addRenderNode( pNode );
	*/
}

//////////////////////////////////////////////////////////////////////////
// addSprite
void RsFrameGL::addSprite( RsMaterial* pMaterial, const BcVec2d& Position )
{
	BcBreakpoint;

	BcUnusedVar( pMaterial );
	BcUnusedVar( Position );

	/*
	BcAssert( pCurrPrimitive_ == NULL );

	// Create instance for rendering.
	RsMaterialPrimitiveNode* pNode = newObject< RsMaterialPrimitiveNode >();

	// Setup vertices.
	TVertex2D* pVertices = alloc< TVertex2D >( 4 );
	
	BcU32 Colour = pMaterial->colour().asARGB();
	
	pVertices[0].X_ = Position.x();
	pVertices[0].Y_ = Position.y();
	pVertices[0].U_ = 0.0f;
	pVertices[0].V_ = 0.0f;
	pVertices[0].Colour_ = Colour;
	
	pVertices[1].X_ = Position.x() + 1.0f;
	pVertices[1].Y_ = Position.y();
	pVertices[1].U_ = 1.0f;
	pVertices[1].V_ = 0.0f;
	pVertices[1].Colour_ = Colour;
	
	pVertices[2].X_ = Position.x();
	pVertices[2].Y_ = Position.y() + 1.0f;
	pVertices[2].U_ = 0.0f;
	pVertices[2].V_ = 1.0f;
	pVertices[2].Colour_ = Colour;
	
	pVertices[3].X_ = Position.x() + 1.0f;
	pVertices[3].Y_ = Position.y() + 1.0f;
	pVertices[3].U_ = 1.0f;
	pVertices[3].V_ = 1.0f;
	pVertices[3].Colour_ = Colour;

	pNode->pEffect_ = RsCore::pImpl()->getDefaultEffect( rsFX_GEO_2D );
	pNode->PrimType_= rsPT_TRIANGLESTRIP;
	pNode->NoofPrims_ = 2;
	pNode->VertexFormat_ = TVertex2D::VERTEX_TYPE;
	pNode->VertexStride_ = sizeof( TVertex2D );
	pNode->pVertices_ = pVertices;

	// Setup sort.
	RsRenderSort Sort;

	Sort.Value_ = pMaterial->sort().Value_;
	Sort.Pass_ = RS_SORT_PASS_OVERLAY;
	pNode->Sort_.Value_ |= Sort.Value_;

	addRenderNode( pNode );
	*/
}

//////////////////////////////////////////////////////////////////////////
// addPrimitive
void RsFrameGL::addPrimitive( RsMaterial* pMaterial, RsEffect* pEffect, eRsPrimitiveType Type, BcU32 NoofPrimitives, BcU32 VertexFormat, const void* pVertices, BcU32 Layer, BcU32 Depth )
{
	BcBreakpoint;

	BcUnusedVar( pMaterial );
	BcUnusedVar( pEffect );
	BcUnusedVar( Type );
	BcUnusedVar( NoofPrimitives );
	BcUnusedVar( VertexFormat );
	BcUnusedVar( pVertices );
	BcUnusedVar( Layer );
	BcUnusedVar( Depth );

	/*
	BcAssert( pCurrPrimitive_ == NULL );

	if( pMaterial != NULL )
	{
		// Create node for rendering.
		RsMaterialPrimitiveNode* pNode = newObject< RsMaterialPrimitiveNode >();

		pNode->pEffect_ = pEffect != NULL ? pEffect : RsCore::pImpl()->getDefaultEffect( rsFX_GEO_3D );
		pNode->pMaterial_ = pMaterial;
		pNode->PrimType_ = Type;
		pNode->NoofPrims_ = NoofPrimitives;
		pNode->VertexFormat_ = VertexFormat;
		pNode->VertexStride_ = RsVertexDeclSize( VertexFormat );
		pNode->pVertices_ = pVertices;

		// Setup sort.
		RsRenderSort Sort;
		Sort = pMaterial->sort();
		Sort.Depth_ = Depth;
		pNode->Sort_.Value_ |= Sort.Value_;

		//
		addRenderNode( pNode );
	}
	else
	{
		// Create node for rendering.
		RsPrimitiveNode* pNode = new( alloc< RsPrimitiveNode >() ) RsPrimitiveNode();

		pNode->pEffect_ = pEffect != NULL ? pEffect : RsCore::pImpl()->getDefaultEffect( rsFX_GEO_3D );
		pNode->PrimType_ = Type;
		pNode->NoofPrims_ = NoofPrimitives;
		pNode->VertexFormat_ = VertexFormat;
		pNode->VertexStride_ = RsVertexDeclSize( VertexFormat );
		pNode->pVertices_ = pVertices;

		// Setup sort.
		RsRenderSort Sort;

		BcAssert( Layer <= RS_SORT_LAYER_MAX );
		Sort.Value_ = 0;
		pNode->Sort_.Depth_ = Depth;
		pNode->Sort_.Blend_ = rsBM_BLEND;
		pNode->Sort_.Pass_ = RS_SORT_PASS_FORWARD;
		pNode->Sort_.Layer_ = Layer;
		pNode->Sort_.Value_ |= Sort.Value_;

		addRenderNode( pNode );
	}
	*/
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
