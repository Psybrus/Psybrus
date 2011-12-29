/**************************************************************************
*
* File:		ScnCanvasComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	10/04/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnCanvasComponent.h"
#include "ScnEntity.h"

#ifdef PSY_SERVER
#include "BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnCanvasComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnCanvasComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );
	
	PropertyTable.beginCatagory( "ScnCanvasComponent" )
		//.field( "source",					csPVT_FILE,			csPCT_LIST )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnCanvasComponent::initialise( BcU32 NoofVertices, ScnMaterialComponentRef DefaultMaterialComponent )
{
	// NULL internals.
	BcMemZero( &RenderResources_[ 0 ], sizeof( RenderResources_ ) );
	HaveVertexBufferLock_ = BcFalse;
	
	// Setup matrix stack with an identity matrix and reserve.
	MatrixStack_.reserve( 16 );
	MatrixStack_.push_back( BcMat4d() );
	IsIdentity_ = BcTrue;
	
	// Store number of vertices.
	NoofVertices_ = NoofVertices;
	
	// Store default material instance.
	DefaultMaterialComponent_ = DefaultMaterialComponent; 
	MaterialComponent_ = DefaultMaterialComponent_;

	// Which render resource to use.
	CurrentRenderResource_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnCanvasComponent::create()
{
	// Allocate our own vertex buffer data.
	BcU32 VertexFormat = rsVDF_POSITION_XYZ | rsVDF_NORMAL_XYZ | rsVDF_TANGENT_XYZ | rsVDF_TEXCOORD_UV0 | rsVDF_COLOUR_RGBA8;
	BcAssert( RsVertexDeclSize( VertexFormat ) == sizeof( ScnCanvasComponentVertex ) );

	// Allocate render resources.
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TRenderResource& RenderResource = RenderResources_[ Idx ];

		// Allocate vertices.
		RenderResource.pVertices_ = new ScnCanvasComponentVertex[ NoofVertices_ ];

		// Allocate render side vertex buffer.
		RenderResource.pVertexBuffer_ = RsCore::pImpl()->createVertexBuffer( VertexFormat, NoofVertices_, RenderResource.pVertices_ );
	
		// Allocate render side primitive.
		RenderResource.pPrimitive_ = RsCore::pImpl()->createPrimitive( RenderResource.pVertexBuffer_, NULL );
	}
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnCanvasComponent::destroy()
{
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TRenderResource& RenderResource = RenderResources_[ Idx ];

		// Allocate render side vertex buffer.
		RsCore::pImpl()->destroyResource( RenderResource.pVertexBuffer_ );
	
		// Allocate render side primitive.
		RsCore::pImpl()->destroyResource( RenderResource.pPrimitive_ );

		// Delete vertices.
		delete [] RenderResource.pVertices_;
	}
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnCanvasComponent::isReady()
{
	// TODO: Just set a sodding flag ok?
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TRenderResource& RenderResource = RenderResources_[ Idx ];

		if( RenderResource.pVertexBuffer_ == NULL || RenderResource.pPrimitive_ == NULL )
		{
			return BcFalse;
		}
	}

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// setMaterialComponent
void ScnCanvasComponent::setMaterialComponent( ScnMaterialComponentRef MaterialComponent )
{
	// Switch material component.
	if( MaterialComponent_ != MaterialComponent )
	{
		// Ensure the material component is attached to an entity (doesn't have to be the same as us)
		BcAssertMsg( MaterialComponent->isAttached() == BcTrue, "Material component is not attached to an entity!" );

		// Cache and grab diffuse parameter.
		MaterialComponent_ = MaterialComponent;
		BcU32 Parameter = MaterialComponent_->findParameter( "aDiffuseTex" );
		DiffuseTexture_ = MaterialComponent_->getTexture( Parameter );
	}
}

//////////////////////////////////////////////////////////////////////////
// pushMatrix
void ScnCanvasComponent::pushMatrix( const BcMat4d& Matrix )
{
	const BcMat4d& CurrMatrix = getMatrix();
	BcMat4d NewMatrix = Matrix * CurrMatrix;
	MatrixStack_.push_back( NewMatrix );
	IsIdentity_ = NewMatrix.isIdentity();
}

//////////////////////////////////////////////////////////////////////////
// popMatrix
void ScnCanvasComponent::popMatrix()
{
	BcAssertMsg( MatrixStack_.size(), "ScnCanvasComponent: Can't pop the last matrix off the stack! Mismatching push/pop?" );
	
	if( MatrixStack_.size() > 1 )
	{
		MatrixStack_.pop_back();
		const BcMat4d& CurrMatrix = getMatrix();
		IsIdentity_ = CurrMatrix.isIdentity();
	}
}

//////////////////////////////////////////////////////////////////////////
// getMatrix
BcMat4d ScnCanvasComponent::getMatrix() const
{
	return MatrixStack_[ MatrixStack_.size() - 1 ];
}

//////////////////////////////////////////////////////////////////////////
// allocVertices
ScnCanvasComponentVertex* ScnCanvasComponent::allocVertices( BcU32 NoofVertices )
{
	BcAssertMsg( HaveVertexBufferLock_ == BcTrue, "ScnCanvasComponent: Don't have vertex buffer lock!" );
	ScnCanvasComponentVertex* pCurrVertex = NULL;
	if( ( VertexIndex_ + NoofVertices ) <= NoofVertices_ )
	{
		pCurrVertex = &pVertices_[ VertexIndex_ ];
		VertexIndex_ += NoofVertices;
	}
	return pCurrVertex;
}

//////////////////////////////////////////////////////////////////////////
// addPrimitive
void ScnCanvasComponent::addPrimitive( eRsPrimitiveType Type, ScnCanvasComponentVertex* pVertices, BcU32 NoofVertices, BcU32 Layer, BcBool UseMatrixStack )
{
	// Check if the vertices are owned by us, if not copy in.
	if( pVertices < pVertices_ || pVertices_ >= pVerticesEnd_ )
	{
		ScnCanvasComponentVertex* pNewVertices = allocVertices( NoofVertices );
		if( pNewVertices != NULL )
		{
			BcMemCopy( pNewVertices, pVertices, sizeof( ScnCanvasComponentVertex ) * NoofVertices );
			pVertices = pNewVertices;
		}
	}
	
	// Matrix stack.
	if( UseMatrixStack == BcTrue && IsIdentity_ == BcFalse )
	{
		BcMat4d Matrix = getMatrix();

		for( BcU32 Idx = 0; Idx < NoofVertices; ++Idx )
		{
			ScnCanvasComponentVertex* pVertex = &pVertices[ Idx ];
			BcVec3d Vertex = BcVec3d( pVertex->X_, pVertex->Y_, pVertex->Z_ ) * Matrix;
			pVertex->X_ = Vertex.x();
			pVertex->Y_ = Vertex.y();
			pVertex->Z_ = Vertex.z();
		}
	}
	
	// TODO: If there was a previous primitive which we can marge into, attempt to.
	BcU32 VertexIndex = convertVertexPointerToIndex( pVertices );
	ScnCanvasComponentPrimitiveSection PrimitiveSection = 
	{
		Type,
		VertexIndex,
		NoofVertices,
		Layer,
		MaterialComponent_
	};
	
	PrimitiveSectionList_.push_back( PrimitiveSection );
	LastPrimitiveSection_ = PrimitiveSectionList_.size() - 1;
}

//////////////////////////////////////////////////////////////////////////
// drawLine
void ScnCanvasComponent::drawLine( const BcVec2d& PointA, const BcVec2d& PointB, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasComponentVertex* pVertices = allocVertices( 2 );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 RGBA = Colour.asABGR();
		
		pVertices->X_ = PointA.x();
		pVertices->Y_ = PointA.y();
		pVertices->Z_ = 0.0f;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		pVertices->X_ = PointB.x();
		pVertices->Y_ = PointB.y();
		pVertices->Z_ = 0.0f;
		pVertices->RGBA_ = RGBA;

		// Add primitive.	
		addPrimitive( rsPT_LINELIST, pFirstVertex, 2, Layer, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// drawLines
void ScnCanvasComponent::drawLines( const BcVec2d* pPoints, BcU32 NoofLines, const RsColour& Colour, BcU32 Layer )
{
	BcU32 NoofVertices = 2 * NoofLines;
	ScnCanvasComponentVertex* pVertices = allocVertices( NoofVertices );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;

	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{	
		// Now copy in data.
		BcU32 RGBA = Colour.asABGR();

		for( BcU32 Idx = 0; Idx < NoofVertices; ++Idx )
		{
			pVertices->X_ = pPoints[ Idx ].x();
			pVertices->Y_ = pPoints[ Idx ].y();
			pVertices->Z_ = 0.0f;
			pVertices->RGBA_ = RGBA;
			++pVertices;
		}
		
		// Add primitive.		
		addPrimitive( rsPT_LINELIST, pFirstVertex, NoofVertices, Layer, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnCanvasComponent::drawBox( const BcVec2d& CornerA, const BcVec2d& CornerB, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasComponentVertex* pVertices = allocVertices( 4 );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 RGBA = Colour.asABGR();
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = 0.0f;
		pVertices->V_ = 0.0f;
		pVertices->RGBA_ = RGBA;
		++pVertices;

		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = 1.0f;
		pVertices->V_ = 0.0f;
		pVertices->RGBA_ = RGBA;
		++pVertices;

		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = 0.0f;
		pVertices->V_ = 1.0f;
		pVertices->RGBA_ = RGBA;
		++pVertices;

		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = 1.0f;
		pVertices->V_ = 1.0f;
		pVertices->RGBA_ = RGBA;
		
		// Add primitive.	
		addPrimitive( rsPT_TRIANGLESTRIP, pFirstVertex, 4, Layer, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// drawSprite
void ScnCanvasComponent::drawSprite( const BcVec2d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasComponentVertex* pVertices = allocVertices( 6 );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;
	
	const BcVec2d CornerA = Position;
	const BcVec2d CornerB = Position + Size;

	const ScnRect Rect = DiffuseTexture_.isValid() ? DiffuseTexture_->getRect( TextureIdx ) : ScnRect();
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 RGBA = Colour.asABGR();
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->RGBA_ = RGBA;
		++pVertices;

		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_;
		pVertices->RGBA_ = RGBA;
		++pVertices;

		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->RGBA_ = RGBA;
		
		// Quickly check last primitive.
		BcBool AddNewPrimitive = BcTrue;
		if( LastPrimitiveSection_ != BcErrorCode )
		{
			ScnCanvasComponentPrimitiveSection& PrimitiveSection = PrimitiveSectionList_[ LastPrimitiveSection_ ];

			// If the last primitive was the same type as ours we can append to it.
			// NOTE: Need more checks here later.
			if( PrimitiveSection.Type_ == rsPT_TRIANGLELIST &&
				PrimitiveSection.Layer_ == Layer &&
				PrimitiveSection.MaterialComponent_ == MaterialComponent_ )
			{
				PrimitiveSection.NoofVertices_ += 6;

				// Matrix stack.
				// TODO: Factor into a seperate function.
				if( IsIdentity_ == BcFalse )
				{
					BcMat4d Matrix = getMatrix();

					for( BcU32 Idx = 0; Idx < 6; ++Idx )
					{
						ScnCanvasComponentVertex* pVertex = &pFirstVertex[ Idx ];
						BcVec3d Vertex = BcVec3d( pVertex->X_, pVertex->Y_, pVertex->Z_ ) * Matrix;
						pVertex->X_ = Vertex.x();
						pVertex->Y_ = Vertex.y();
						pVertex->Z_ = Vertex.z();
					}
				}
				
				AddNewPrimitive = BcFalse;
			}
		}
		
		// Add primitive.
		if( AddNewPrimitive == BcTrue )
		{
			addPrimitive( rsPT_TRIANGLELIST, pFirstVertex, 6, Layer, BcTrue );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// drawSprite
void ScnCanvasComponent::drawSprite3D( const BcVec3d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasComponentVertex* pVertices = allocVertices( 6 );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;
	
	const BcVec3d CornerA = Position;
	const BcVec3d CornerB = Position + BcVec3d( Size.x(), Size.y(), 0.0f );
	
	const ScnRect Rect = DiffuseTexture_.isValid() ? DiffuseTexture_->getRect( TextureIdx ) : ScnRect();
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 RGBA = Colour.asABGR();
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = Position.z();
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = Position.z();
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = Position.z();
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = Position.z();
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = Position.z();
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = Position.z();
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->RGBA_ = RGBA;
		
		// Quickly check last primitive.
		BcBool AddNewPrimitive = BcTrue;
		if( LastPrimitiveSection_ != BcErrorCode )
		{
			ScnCanvasComponentPrimitiveSection& PrimitiveSection = PrimitiveSectionList_[ LastPrimitiveSection_ ];
			
			// If the last primitive was the same type as ours we can append to it.
			// NOTE: Need more checks here later.
			if( PrimitiveSection.Type_ == rsPT_TRIANGLELIST &&
			   PrimitiveSection.Layer_ == Layer &&
			   PrimitiveSection.MaterialComponent_ == MaterialComponent_ )
			{
				PrimitiveSection.NoofVertices_ += 6;
				
				// Matrix stack.
				// TODO: Factor into a seperate function.
				if( IsIdentity_ == BcFalse )
				{
					BcMat4d Matrix = getMatrix();
					
					for( BcU32 Idx = 0; Idx < 6; ++Idx )
					{
						ScnCanvasComponentVertex* pVertex = &pFirstVertex[ Idx ];
						BcVec3d Vertex = BcVec3d( pVertex->X_, pVertex->Y_, pVertex->Z_ ) * Matrix;
						pVertex->X_ = Vertex.x();
						pVertex->Y_ = Vertex.y();
						pVertex->Z_ = Vertex.z();
					}
				}
				
				AddNewPrimitive = BcFalse;
			}
		}
		
		// Add primitive.
		if( AddNewPrimitive == BcTrue )
		{
			addPrimitive( rsPT_TRIANGLELIST, pFirstVertex, 6, Layer, BcTrue );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// drawSprite
void ScnCanvasComponent::drawSpriteUp3D( const BcVec3d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasComponentVertex* pVertices = allocVertices( 6 );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;
	
	const BcVec3d CornerA = Position;
	const BcVec3d CornerB = Position + BcVec3d( Size.x(), 0.0f, Size.y() );
	
	const ScnRect Rect = DiffuseTexture_.isValid() ? DiffuseTexture_->getRect( TextureIdx ) : ScnRect();
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 RGBA = Colour.asABGR();
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = Position.y();
		pVertices->Z_ = CornerA.z();
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = Position.y();
		pVertices->Z_ = CornerA.z();
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = Position.y();
		pVertices->Z_ = CornerB.z();
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = Position.y();
		pVertices->Z_ = CornerB.z();
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = Position.y();
		pVertices->Z_ = CornerA.z();
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = Position.y();
		pVertices->Z_ = CornerB.z();
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->RGBA_ = RGBA;
		
		// Quickly check last primitive.
		BcBool AddNewPrimitive = BcTrue;
		if( LastPrimitiveSection_ != BcErrorCode )
		{
			ScnCanvasComponentPrimitiveSection& PrimitiveSection = PrimitiveSectionList_[ LastPrimitiveSection_ ];
			
			// If the last primitive was the same type as ours we can append to it.
			// NOTE: Need more checks here later.
			if( PrimitiveSection.Type_ == rsPT_TRIANGLELIST &&
			   PrimitiveSection.Layer_ == Layer &&
			   PrimitiveSection.MaterialComponent_ == MaterialComponent_ )
			{
				PrimitiveSection.NoofVertices_ += 6;
				
				// Matrix stack.
				// TODO: Factor into a seperate function.
				if( IsIdentity_ == BcFalse )
				{
					BcMat4d Matrix = getMatrix();
					
					for( BcU32 Idx = 0; Idx < 6; ++Idx )
					{
						ScnCanvasComponentVertex* pVertex = &pFirstVertex[ Idx ];
						BcVec3d Vertex = BcVec3d( pVertex->X_, pVertex->Y_, pVertex->Z_ ) * Matrix;
						pVertex->X_ = Vertex.x();
						pVertex->Y_ = Vertex.y();
						pVertex->Z_ = Vertex.z();
					}
				}
				
				AddNewPrimitive = BcFalse;
			}
		}
		
		// Add primitive.
		if( AddNewPrimitive == BcTrue )
		{
			addPrimitive( rsPT_TRIANGLELIST, pFirstVertex, 6, Layer, BcTrue );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// drawSpriteCentered
void ScnCanvasComponent::drawSpriteCentered( const BcVec2d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	BcVec2d NewPosition = Position - ( Size * 0.5f );
	drawSprite( NewPosition, Size, TextureIdx, Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// drawSpriteCentered
void ScnCanvasComponent::drawSpriteCentered3D( const BcVec3d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	BcVec3d NewPosition = Position - BcVec3d( Size.x() * 0.5f, Size.y() * 0.5f, 0.0f );
	drawSprite3D( NewPosition, Size, TextureIdx, Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// drawSpriteCentered
void ScnCanvasComponent::drawSpriteCenteredUp3D( const BcVec3d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	BcVec3d NewPosition = Position - BcVec3d( Size.x() * 0.5f, 0.0f, Size.y() * 0.5f );
	drawSpriteUp3D( NewPosition, Size, TextureIdx, Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnCanvasComponent::clear()
{
	// Set current render resource.
	pRenderResource_ = &RenderResources_[ CurrentRenderResource_ ];

	// Set vertices up.
	pVertices_ = pVerticesEnd_ = pRenderResource_->pVertices_;
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
		pRenderResource_->pVertexBuffer_->lock();
		HaveVertexBufferLock_ = BcTrue;
	}

	// Clear last primitive.
	LastPrimitiveSection_ = BcErrorCode;

	// Set material instance to default material instance.
	setMaterialComponent( DefaultMaterialComponent_ );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnCanvasComponent::update( BcReal Tick )
{
	Super::update( Tick );

	clear(); // Temporary hack. Need to fix this.
}

//////////////////////////////////////////////////////////////////////////
// render
class ScnCanvasComponentRenderNode: public RsRenderNode
{
public:
	void render()
	{
		// TODO: Cache material instance so we don't rebind?
		for( BcU32 Idx = 0; Idx < NoofSections_; ++Idx )
		{
			ScnCanvasComponentPrimitiveSection* pPrimitiveSection = &pPrimitiveSections_[ Idx ];
			
			pPrimitive_->render( pPrimitiveSection->Type_, pPrimitiveSection->VertexIndex_, pPrimitiveSection->NoofVertices_ );
		}
	}
	
	BcU32 NoofSections_;
	ScnCanvasComponentPrimitiveSection* pPrimitiveSections_;
	RsPrimitive* pPrimitive_;
};

void ScnCanvasComponent::render( RsFrame* pFrame, RsRenderSort Sort )
{
	BcAssertMsg( HaveVertexBufferLock_ == BcTrue, "ScnCanvasComponent: Can't render without a vertex buffer lock." );

	// NOTE: Could do this sort inside of the renderer, but I'm just gonna keep the canvas
	//       as one solid object as to not conflict with other canvas objects when rendered
	//       to the scene. Will not sort by transparency or anything either.
	std::stable_sort( PrimitiveSectionList_.begin(), PrimitiveSectionList_.end(), ScnCanvasComponentPrimitiveSectionCompare() );
	
	// Cache last material instance.
	ScnMaterialComponent* pLastMaterialComponent = NULL;
	
	for( BcU32 Idx = 0; Idx < PrimitiveSectionList_.size(); ++Idx )
	{
		ScnCanvasComponentRenderNode* pRenderNode = pFrame->newObject< ScnCanvasComponentRenderNode >();
	
		pRenderNode->NoofSections_ = 1;//PrimitiveSectionList_.size();
		pRenderNode->pPrimitiveSections_ = pFrame->alloc< ScnCanvasComponentPrimitiveSection >( 1 );
		pRenderNode->pPrimitive_ = pRenderResource_->pPrimitive_;
		
		// Copy primitive sections in.
		BcMemCopy( pRenderNode->pPrimitiveSections_, &PrimitiveSectionList_[ Idx ], sizeof( ScnCanvasComponentPrimitiveSection ) * 1 );

		// Bind material.
		if( pLastMaterialComponent != pRenderNode->pPrimitiveSections_->MaterialComponent_ )
		{
			pLastMaterialComponent = pRenderNode->pPrimitiveSections_->MaterialComponent_;
			pLastMaterialComponent->bind( pFrame, Sort );
		}

		// Add to frame.
		pRenderNode->Sort_ = Sort;
		pFrame->addRenderNode( pRenderNode );
	}
	
	// Unlock vertex buffer.
	pRenderResource_->pVertexBuffer_->setNoofUpdateVertices( VertexIndex_ );
	pRenderResource_->pVertexBuffer_->unlock();
	HaveVertexBufferLock_ = BcFalse;

	// Flip the render resource.
	CurrentRenderResource_ = 1 - CurrentRenderResource_;

	// Reset render resource pointers to aid debugging.
	pRenderResource_ = NULL;
	pVertices_ = pVerticesEnd_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnCanvasComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );	
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnCanvasComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}
