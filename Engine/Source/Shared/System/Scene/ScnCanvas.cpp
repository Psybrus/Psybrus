/**************************************************************************
*
* File:		ScnCanvas.cpp
* Author:	Neil Richardson 
* Ver/Date:	10/04/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnCanvas.h"


#ifdef PSY_SERVER
#include "BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnCanvas );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnCanvas::initialise( BcU32 NoofVertices, ScnMaterialInstanceRef DefaultMaterialInstance )
{
	// NULL internals.
	pVertexBuffer_ = NULL;
	HaveVertexBufferLock_ = BcFalse;
	
	// Setup matrix stack with an identity matrix and reserve.
	MatrixStack_.reserve( 16 );
	MatrixStack_.push_back( BcMat4d() );
	IsIdentity_ = BcTrue;
	
	// Store number of vertices.
	NoofVertices_ = NoofVertices;
	
	// Store default material instance.
	DefaultMaterialInstance_ = DefaultMaterialInstance; 
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnCanvas::create()
{
	// Allocate our own vertex buffer data.
	BcU32 VertexFormat = rsVDF_POSITION_XY | rsVDF_TEXCOORD_UV0 | rsVDF_COLOUR_RGBA8;
	BcAssert( RsVertexDeclSize( VertexFormat ) == sizeof( ScnCanvasVertex ) );
	pVertices_ = new ScnCanvasVertex[ NoofVertices_ ];
	pVerticesEnd_ = pVertices_ + NoofVertices_;
	VertexIndex_ = 0;
	
	// Allocate render side vertex buffer.
	pVertexBuffer_ = RsCore::pImpl()->createVertexBuffer( rsVDF_POSITION_XY | rsVDF_TEXCOORD_UV0 | rsVDF_COLOUR_RGBA8, NoofVertices_, pVertices_ );
	
	// Allocate render side primitive.
	pPrimitive_ = RsCore::pImpl()->createPrimitive( pVertexBuffer_, NULL );
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnCanvas::destroy()
{
	// Destroy our primitive.
	RsCore::pImpl()->destroyResource( pPrimitive_ );
	
	// Destroy our vertex buffer.
	RsCore::pImpl()->destroyResource( pVertexBuffer_ );
	
	delete [] pVertices_;
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnCanvas::isReady()
{
	return pVertexBuffer_ != NULL && pPrimitive_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// setMaterialInstance
void ScnCanvas::setMaterialInstance( ScnMaterialInstanceRef MaterialInstance )
{
	MaterialInstance_ = MaterialInstance;
	DiffuseTexture_ = MaterialInstance_->getTexture( 0 );
}

//////////////////////////////////////////////////////////////////////////
// pushMatrix
void ScnCanvas::pushMatrix( const BcMat4d& Matrix )
{
	const BcMat4d& CurrMatrix = getMatrix();
	BcMat4d NewMatrix = Matrix * CurrMatrix;
	MatrixStack_.push_back( NewMatrix );
	IsIdentity_ = NewMatrix.isIdentity();
}

//////////////////////////////////////////////////////////////////////////
// popMatrix
void ScnCanvas::popMatrix()
{
	BcAssertMsg( MatrixStack_.size(), "ScnCanvas: Can't pop the last matrix off the stack! Mismatching push/pop?" );
	
	if( MatrixStack_.size() > 1 )
	{
		MatrixStack_.pop_back();
		const BcMat4d& CurrMatrix = getMatrix();
		IsIdentity_ = CurrMatrix.isIdentity();
	}
}

//////////////////////////////////////////////////////////////////////////
// getMatrix
BcMat4d ScnCanvas::getMatrix() const
{
	return MatrixStack_[ MatrixStack_.size() - 1 ];
}

//////////////////////////////////////////////////////////////////////////
// allocVertices
ScnCanvasVertex* ScnCanvas::allocVertices( BcU32 NoofVertices )
{
	BcAssertMsg( HaveVertexBufferLock_ == BcTrue, "ScnCanvas: Don't have vertex buffer lock!" );
	ScnCanvasVertex* pCurrVertex = NULL;
	if( ( VertexIndex_ + NoofVertices ) <= NoofVertices_ )
	{
		pCurrVertex = &pVertices_[ VertexIndex_ ];
		VertexIndex_ += NoofVertices;
	}
	return pCurrVertex;
}

//////////////////////////////////////////////////////////////////////////
// addPrimitive
void ScnCanvas::addPrimitive( eRsPrimitiveType Type, ScnCanvasVertex* pVertices, BcU32 NoofVertices, BcU32 Layer, BcBool UseMatrixStack )
{
	// Check if the vertices are owned by us, if not copy in.
	if( pVertices < pVertices_ || pVertices_ >= pVerticesEnd_ )
	{
		ScnCanvasVertex* pNewVertices = allocVertices( NoofVertices );
		if( pNewVertices != NULL )
		{
			BcMemCopy( pNewVertices, pVertices, sizeof( ScnCanvasVertex ) * NoofVertices );
			pVertices = pNewVertices;
		}
	}
	
	// Matrix stack.
	if( UseMatrixStack == BcTrue && IsIdentity_ == BcFalse )
	{
		BcMat4d Matrix = getMatrix();

		for( BcU32 Idx = 0; Idx < NoofVertices; ++Idx )
		{
			ScnCanvasVertex* pVertex = &pVertices[ Idx ];
			BcVec2d Vertex = BcVec2d( pVertex->X_, pVertex->Y_ ) * Matrix;
			pVertex->X_ = Vertex.x();
			pVertex->Y_ = Vertex.y();
		}
	}
	
	// TODO: If there was a previous primitive which we can marge into, attempt to.
	BcU32 VertexIndex = convertVertexPointerToIndex( pVertices );
	ScnCanvasPrimitiveSection PrimitiveSection = 
	{
		Type,
		VertexIndex,
		NoofVertices,
		Layer,
		MaterialInstance_
	};
	
	PrimitiveSectionList_.push_back( PrimitiveSection );
}

//////////////////////////////////////////////////////////////////////////
// drawLine
void ScnCanvas::drawLine( const BcVec2d& PointA, const BcVec2d& PointB, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasVertex* pVertices = allocVertices( 2 );
	ScnCanvasVertex* pFirstVertex = pVertices;
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 RGBA = Colour.asARGB();
		
		pVertices->X_ = PointA.x();
		pVertices->Y_ = PointA.y();
		pVertices->RGBA_ = RGBA;
		++pVertices;
		pVertices->X_ = PointB.x();
		pVertices->Y_ = PointB.y();
		pVertices->RGBA_ = RGBA;

		// Add primitive.	
		addPrimitive( rsPT_LINELIST, pFirstVertex, 2, Layer, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// drawLines
void ScnCanvas::drawLines( const BcVec2d* pPoints, BcU32 NoofLines, const RsColour& Colour, BcU32 Layer )
{
	BcU32 NoofVertices = 2 * NoofLines;
	ScnCanvasVertex* pVertices = allocVertices( NoofVertices );
	ScnCanvasVertex* pFirstVertex = pVertices;

	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{	
		// Now copy in data.
		BcU32 RGBA = Colour.asARGB();

		for( BcU32 Idx = 0; Idx < NoofVertices; ++Idx )
		{
			pVertices->X_ = pPoints[ Idx ].x();
			pVertices->Y_ = pPoints[ Idx ].y();
			pVertices->RGBA_ = RGBA;
			++pVertices;
		}
		
		// Add primitive.		
		addPrimitive( rsPT_LINELIST, pFirstVertex, NoofVertices, Layer, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnCanvas::drawBox( const BcVec2d& CornerA, const BcVec2d& CornerB, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasVertex* pVertices = allocVertices( 4 );
	ScnCanvasVertex* pFirstVertex = pVertices;
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 RGBA = Colour.asARGB();
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerA.y();
		pVertices->U_ = 0.0f;
		pVertices->V_ = 0.0f;
		pVertices->RGBA_ = RGBA;
		++pVertices;

		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerA.y();
		pVertices->U_ = 1.0f;
		pVertices->V_ = 0.0f;
		pVertices->RGBA_ = RGBA;
		++pVertices;

		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerB.y();
		pVertices->U_ = 0.0f;
		pVertices->V_ = 1.0f;
		pVertices->RGBA_ = RGBA;
		++pVertices;

		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerB.y();
		pVertices->U_ = 1.0f;
		pVertices->V_ = 1.0f;
		pVertices->RGBA_ = RGBA;
		
		// Add primitive.	
		addPrimitive( rsPT_TRIANGLESTRIP, pFirstVertex, 4, Layer, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// drawSprite
void ScnCanvas::drawSprite( const BcVec2d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasVertex* pVertices = allocVertices( 4 );
	ScnCanvasVertex* pFirstVertex = pVertices;
	
	const BcVec2d CornerA = Position;
	const BcVec2d CornerB = Position + Size;
	
	// Only render if we have a valid diffuse texture to reference for UV rects.
	if( DiffuseTexture_.isValid() )
	{
		const ScnRect Rect = DiffuseTexture_->getRect( TextureIdx );
	
		// Only draw if we can allocate vertices.
		if( pVertices != NULL )
		{
			// Now copy in data.
			BcU32 RGBA = Colour.asARGB();
		
			pVertices->X_ = CornerA.x();
			pVertices->Y_ = CornerA.y();
			pVertices->U_ = Rect.X_;
			pVertices->V_ = Rect.Y_;
			pVertices->RGBA_ = RGBA;
			++pVertices;
			
			pVertices->X_ = CornerB.x();
			pVertices->Y_ = CornerA.y();
			pVertices->U_ = Rect.X_ + Rect.W_;
			pVertices->V_ = Rect.Y_;
			pVertices->RGBA_ = RGBA;
			++pVertices;
			
			pVertices->X_ = CornerA.x();
			pVertices->Y_ = CornerB.y();
			pVertices->U_ = Rect.X_;
			pVertices->V_ = Rect.Y_ + Rect.H_;
			pVertices->RGBA_ = RGBA;
			++pVertices;
			
			pVertices->X_ = CornerB.x();
			pVertices->Y_ = CornerB.y();
			pVertices->U_ = Rect.X_ + Rect.W_;
			pVertices->V_ = Rect.Y_ + Rect.H_;
			pVertices->RGBA_ = RGBA;
			
			// Add primitive.	
			addPrimitive( rsPT_TRIANGLESTRIP, pFirstVertex, 4, Layer, BcTrue );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// drawSpriteCentered
void ScnCanvas::drawSpriteCentered( const BcVec2d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	BcVec2d NewPosition = Position - ( Size * 0.5f );
	drawSprite( NewPosition, Size, TextureIdx, Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnCanvas::clear()
{
	// Set vertex back to the start.
	VertexIndex_ = 0;
	
	// Empty primitive sections.
	PrimitiveSectionList_.clear();
	
	// Reset matrix stack.
	MatrixStack_.clear();
	MatrixStack_.push_back( BcMat4d() );
	IsIdentity_ = BcTrue;

	// Lock vertex buffer for use.
	if( HaveVertexBufferLock_ == BcFalse )
	{
		pVertexBuffer_->lock();
		HaveVertexBufferLock_ = BcTrue;
	}
	
	// Set material instance to default material instance.
	setMaterialInstance( DefaultMaterialInstance_ );
}

//////////////////////////////////////////////////////////////////////////
// render
class ScnCanvasRenderNode: public RsRenderNode
{
public:
	void render()
	{
		// TODO: Cache material instance so we don't rebind?
		for( BcU32 Idx = 0; Idx < NoofSections_; ++Idx )
		{
			ScnCanvasPrimitiveSection* pPrimitiveSection = &pPrimitiveSections_[ Idx ];
			
			pPrimitive_->render( pPrimitiveSection->Type_, pPrimitiveSection->VertexIndex_, pPrimitiveSection->NoofVertices_ );
		}
	}
	
	BcU32 NoofSections_;
	ScnCanvasPrimitiveSection* pPrimitiveSections_;
	RsPrimitive* pPrimitive_;
};

void ScnCanvas::render( RsFrame* pFrame, RsRenderSort Sort )
{
	// NOTE: Could do this sort inside of the renderer, but I'm just gonna keep the canvas
	//       as one solid object as to not conflict with other canvas objects when rendered
	//       to the scene. Will not sort by transparency or anything either.
	std::stable_sort( PrimitiveSectionList_.begin(), PrimitiveSectionList_.end(), ScnCanvasPrimitiveSectionCompare() );
	
	for( BcU32 Idx = 0; Idx < PrimitiveSectionList_.size(); ++Idx )
	{
		ScnCanvasRenderNode* pRenderNode = pFrame->newObject< ScnCanvasRenderNode >();
	
		pRenderNode->NoofSections_ = 1;//PrimitiveSectionList_.size();
		pRenderNode->pPrimitiveSections_ = pFrame->alloc< ScnCanvasPrimitiveSection >( 1 );
		pRenderNode->pPrimitive_ = pPrimitive_;
		
		// Copy primitive sections in.
		BcMemCopy( pRenderNode->pPrimitiveSections_, &PrimitiveSectionList_[ Idx ], sizeof( ScnCanvasPrimitiveSection ) * 1 );

		// Bind material.
		pRenderNode->pPrimitiveSections_->MaterialInstance_->bind( pFrame, Sort );
		
		// Add to frame.
		pRenderNode->Sort_ = Sort;
		pFrame->addRenderNode( pRenderNode );
	}
	
	// Unlock vertex buffer if we have the lock.
	if( HaveVertexBufferLock_ == BcTrue )
	{
		pVertexBuffer_->unlock();
	}
}
