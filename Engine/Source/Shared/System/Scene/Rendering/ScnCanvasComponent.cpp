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

#include "System/Scene/Rendering/ScnCanvasComponent.h"
#include "System/Scene/ScnEntity.h"

#ifdef PSY_SERVER
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnCanvasComponent );

void ScnCanvasComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "HaveVertexBufferLock_", &ScnCanvasComponent::HaveVertexBufferLock_, bcRFF_TRANSIENT ),
		new ReField( "NoofVertices_", &ScnCanvasComponent::NoofVertices_ ),
		new ReField( "VertexIndex_", &ScnCanvasComponent::VertexIndex_ ),
		new ReField( "MaterialComponent_", &ScnCanvasComponent::MaterialComponent_ ),
		new ReField( "DiffuseTexture_", &ScnCanvasComponent::DiffuseTexture_, bcRFF_TRANSIENT ),
		new ReField( "MatrixStack_", &ScnCanvasComponent::MatrixStack_ ),
		new ReField( "IsIdentity_", &ScnCanvasComponent::IsIdentity_ ),
	};
		
	ReRegisterClass< ScnCanvasComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -2000 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnCanvasComponent::initialise()
{
	initialise( 0 );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnCanvasComponent::initialise( BcU32 NoofVertices )
{
	Super::initialise();

	// Clear render resource to null.
	BcMemZero( &RenderResource_, sizeof( RenderResource_ ) );
	HaveVertexBufferLock_ = BcFalse;

	// Setup matrix stack with an identity matrix and reserve.
	MatrixStack_.reserve( 16 );
	MatrixStack_.push_back( MaMat4d() );
	IsIdentity_ = BcTrue;

	// Store number of vertices.
	pVertices_ = pVerticesEnd_ = nullptr;
	pWorkingVertices_ = nullptr;
	VertexIndex_ = 0;
	VertexDeclaration_ = nullptr;
	NoofVertices_ = NoofVertices;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnCanvasComponent::initialise( const Json::Value& Object )
{
	ScnCanvasComponent::initialise( Object[ "noofvertices" ].asUInt() );
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnCanvasComponent::create()
{
	// Allocate our own vertex buffer data.
	VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration( 
		RsVertexDeclarationDesc( 3 )
			.addElement( RsVertexElement( 0, 0,				4,		RsVertexDataType::FLOAT32,		RsVertexUsage::POSITION,		0 ) )
			.addElement( RsVertexElement( 0, 16,			2,		RsVertexDataType::FLOAT32,		RsVertexUsage::TEXCOORD,		0 ) )
			.addElement( RsVertexElement( 0, 24,			4,		RsVertexDataType::UBYTE_NORM,	RsVertexUsage::COLOUR,			0 ) ) );

	// Allocate render resources.
	RenderResource_.pVertexBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc( 
			RsBufferType::VERTEX,
			RsResourceCreationFlags::STREAM,
			NoofVertices_ * sizeof( ScnCanvasComponentVertex ) ) );

	// Allocate working vertices.
	pWorkingVertices_ = new ScnCanvasComponentVertex[ NoofVertices_ ];

	Super::create();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnCanvasComponent::destroy()
{
	UploadFence_.wait();

	// Allocate render side vertex buffer.
	RsCore::pImpl()->destroyResource( RenderResource_.pVertexBuffer_ );

	// Destroy vertex declaration.
	RsCore::pImpl()->destroyResource( VertexDeclaration_ );

	// Delete working data.
	delete [] pWorkingVertices_;
}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
MaAABB ScnCanvasComponent::getAABB() const
{
	return MaAABB();
}

//////////////////////////////////////////////////////////////////////////
// setMaterialComponent
void ScnCanvasComponent::setMaterialComponent( ScnMaterialComponentRef MaterialComponent )
{
	// Switch material component.
	if( MaterialComponent_ != MaterialComponent )
	{
		// Ensure the material component is attached to an entity (doesn't have to be the same as us)
		BcAssertMsg( MaterialComponent->isAttached() == BcTrue, "Material component \"%s\" (Parent \"%s\") is not attached to an entity! Can't use.", (*MaterialComponent->getName()).c_str(),(*MaterialComponent->getMaterial()->getName()).c_str() );

		// Cache and grab diffuse parameter.
		static BcName NameDiffuseTex( "aDiffuseTex" );
		MaterialComponent_ = MaterialComponent;
		BcU32 Parameter = MaterialComponent_->findTextureSlot( NameDiffuseTex );
		DiffuseTexture_ = MaterialComponent_->getTexture( Parameter );
	}
}

//////////////////////////////////////////////////////////////////////////
// getMaterialComponent
ScnMaterialComponentRef ScnCanvasComponent::getMaterialComponent()
{
	return MaterialComponent_;
}

//////////////////////////////////////////////////////////////////////////
// pushMatrix
void ScnCanvasComponent::pushMatrix( const MaMat4d& Matrix )
{
	const MaMat4d& CurrMatrix = getMatrix();
	MaMat4d NewMatrix = Matrix * CurrMatrix;
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
		const MaMat4d& CurrMatrix = getMatrix();
		IsIdentity_ = CurrMatrix.isIdentity();
	}
}

//////////////////////////////////////////////////////////////////////////
// getMatrix
MaMat4d ScnCanvasComponent::getMatrix() const
{
	return MatrixStack_[ MatrixStack_.size() - 1 ];
}

//////////////////////////////////////////////////////////////////////////
// allocVertices
ScnCanvasComponentVertex* ScnCanvasComponent::allocVertices( BcSize NoofVertices )
{
	BcAssertMsg( pVertices_ != nullptr, "ScnCanvasComponent: Don't have a working buffer." );
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
void ScnCanvasComponent::addPrimitive( RsTopologyType Type, ScnCanvasComponentVertex* pVertices, BcU32 NoofVertices, BcU32 Layer, BcBool UseMatrixStack )
{
	BcAssertMsg( MaterialComponent_.isValid(), "ScnCanvasComponent: Material component has not been set!" );

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
		MaMat4d Matrix = getMatrix();

		for( BcU32 Idx = 0; Idx < NoofVertices; ++Idx )
		{
			ScnCanvasComponentVertex* pVertex = &pVertices[ Idx ];
			MaVec3d Vertex = MaVec3d( pVertex->X_, pVertex->Y_, pVertex->Z_ ) * Matrix;
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
	LastPrimitiveSection_ = (BcU32)PrimitiveSectionList_.size() - 1;
}

//////////////////////////////////////////////////////////////////////////
// drawLine
void ScnCanvasComponent::drawLine( const MaVec2d& PointA, const MaVec2d& PointB, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasComponentVertex* pVertices = allocVertices( 2 );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 ABGR = Colour.asABGR();
		
		pVertices->X_ = PointA.x();
		pVertices->Y_ = PointA.y();
		pVertices->Z_ = 0.0f;
		pVertices->W_ = 1.0f;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		pVertices->X_ = PointB.x();
		pVertices->Y_ = PointB.y();
		pVertices->Z_ = 0.0f;
		pVertices->W_ = 1.0f;
		pVertices->ABGR_ = ABGR;

		// Quickly check last primitive.
		BcBool AddNewPrimitive = BcTrue;
		if( LastPrimitiveSection_ != BcErrorCode )
		{
			ScnCanvasComponentPrimitiveSection& PrimitiveSection = PrimitiveSectionList_[ LastPrimitiveSection_ ];

			// If the last primitive was the same type as ours we can append to it.
			// NOTE: Need more checks here later.
			if( PrimitiveSection.Type_ == RsTopologyType::LINE_LIST &&
				PrimitiveSection.Layer_ == Layer &&
				PrimitiveSection.MaterialComponent_ == MaterialComponent_ )
			{
				PrimitiveSection.NoofVertices_ += 2;

				// Matrix stack.
				// TODO: Factor into a seperate function.
				if( IsIdentity_ == BcFalse )
				{
					MaMat4d Matrix = getMatrix();

					for( BcU32 Idx = 0; Idx < 2; ++Idx )
					{
						ScnCanvasComponentVertex* pVertex = &pFirstVertex[ Idx ];
						MaVec3d Vertex = MaVec3d( pVertex->X_, pVertex->Y_, pVertex->Z_ ) * Matrix;
						pVertex->X_ = Vertex.x();
						pVertex->Y_ = Vertex.y();
						pVertex->Z_ = Vertex.z();
						pVertices->W_ = 1.0f;
					}
				}
				
				AddNewPrimitive = BcFalse;
			}
		}
		
		// Add primitive.
		if( AddNewPrimitive == BcTrue )
		{
			addPrimitive( RsTopologyType::LINE_LIST, pFirstVertex, 2, Layer, BcTrue );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// drawLine3d
void ScnCanvasComponent::drawLine3d( const MaVec3d& PointA, const MaVec3d& PointB, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasComponentVertex* pVertices = allocVertices( 2 );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 ABGR = Colour.asABGR();
		
		pVertices->X_ = PointA.x();
		pVertices->Y_ = PointA.y();
		pVertices->Z_ = PointA.z();
		pVertices->W_ = 1.0f;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		pVertices->X_ = PointB.x();
		pVertices->Y_ = PointB.y();
		pVertices->Z_ = PointB.z();
		pVertices->W_ = 1.0f;
		pVertices->ABGR_ = ABGR;

		// Add primitive.	
		addPrimitive( RsTopologyType::LINE_LIST, pFirstVertex, 2, Layer, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// drawLines
void ScnCanvasComponent::drawLines( const MaVec2d* pPoints, BcU32 NoofLines, const RsColour& Colour, BcU32 Layer )
{
	BcU32 NoofVertices = 2 * NoofLines;
	ScnCanvasComponentVertex* pVertices = allocVertices( NoofVertices );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;

	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{	
		// Now copy in data.
		BcU32 ABGR = Colour.asABGR();

		for( BcU32 Idx = 0; Idx < NoofVertices; ++Idx )
		{
			pVertices->X_ = pPoints[ Idx ].x();
			pVertices->Y_ = pPoints[ Idx ].y();
			pVertices->Z_ = 0.0f;
			pVertices->W_ = 1.0f;
			pVertices->ABGR_ = ABGR;
			++pVertices;
		}
		
		// Add primitive.		
		addPrimitive( RsTopologyType::LINE_LIST, pFirstVertex, NoofVertices, Layer, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// drawLineBox
void ScnCanvasComponent::drawLineBox( const MaVec2d& CornerA, const MaVec2d& CornerB, const RsColour& Colour, BcU32 Layer )
{
	// SLOW.
	drawLine( MaVec2d( CornerA.x(), CornerA.y() ), MaVec2d( CornerB.x(), CornerA.y() ), Colour, Layer );
	drawLine( MaVec2d( CornerB.x(), CornerA.y() ), MaVec2d( CornerB.x(), CornerB.y() ), Colour, Layer );
	drawLine( MaVec2d( CornerB.x(), CornerB.y() ), MaVec2d( CornerA.x(), CornerB.y() ), Colour, Layer );
	drawLine( MaVec2d( CornerA.x(), CornerB.y() ), MaVec2d( CornerA.x(), CornerA.y() ), Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// drawLineBox
void ScnCanvasComponent::drawLineBoxCentered( const MaVec2d& Position, const MaVec2d& Size, const RsColour& Colour, BcU32 Layer )
{
	MaVec2d HalfSize = Size * 0.5f;
	MaVec2d CornerA = Position - HalfSize;
	MaVec2d CornerB = Position + HalfSize;
	drawLineBox( CornerA, CornerB, Colour, Layer );
}


//////////////////////////////////////////////////////////////////////////
// drawBox
void ScnCanvasComponent::drawBox( const MaVec2d& CornerA, const MaVec2d& CornerB, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasComponentVertex* pVertices = allocVertices( 4 );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 ABGR = Colour.asABGR();
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = 0.0f;
		pVertices->W_ = 1.0f;
		pVertices->U_ = 0.0f;
		pVertices->V_ = 0.0f;
		pVertices->ABGR_ = ABGR;
		++pVertices;

		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = 0.0f;
		pVertices->W_ = 1.0f;
		pVertices->U_ = 1.0f;
		pVertices->V_ = 0.0f;
		pVertices->ABGR_ = ABGR;
		++pVertices;

		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = 0.0f;
		pVertices->W_ = 1.0f;
		pVertices->U_ = 0.0f;
		pVertices->V_ = 1.0f;
		pVertices->ABGR_ = ABGR;
		++pVertices;

		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = 0.0f;
		pVertices->W_ = 1.0f;
		pVertices->U_ = 1.0f;
		pVertices->V_ = 1.0f;
		pVertices->ABGR_ = ABGR;
		
		// Add primitive.	
		addPrimitive( RsTopologyType::TRIANGLE_STRIP, pFirstVertex, 4, Layer, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// drawSprite
void ScnCanvasComponent::drawSprite( const MaVec2d& Position, const MaVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasComponentVertex* pVertices = allocVertices( 6 );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;
	
	const MaVec2d CornerA = Position;
	const MaVec2d CornerB = Position + Size;

	const ScnRect Rect = DiffuseTexture_.isValid() ? DiffuseTexture_->getRect( TextureIdx ) : ScnRect();
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 ABGR = Colour.asABGR();
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = 0.0f;
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = 0.0f;
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = 0.0f;
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = 0.0f;
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->ABGR_ = ABGR;
		++pVertices;

		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = 0.0f;
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_;
		pVertices->ABGR_ = ABGR;
		++pVertices;

		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = 0.0f;
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->ABGR_ = ABGR;
		
		// Quickly check last primitive.
		BcBool AddNewPrimitive = BcTrue;
		if( LastPrimitiveSection_ != BcErrorCode )
		{
			ScnCanvasComponentPrimitiveSection& PrimitiveSection = PrimitiveSectionList_[ LastPrimitiveSection_ ];

			// If the last primitive was the same type as ours we can append to it.
			// NOTE: Need more checks here later.
			if( PrimitiveSection.Type_ == RsTopologyType::TRIANGLE_LIST &&
				PrimitiveSection.Layer_ == Layer &&
				PrimitiveSection.MaterialComponent_ == MaterialComponent_ )
			{
				PrimitiveSection.NoofVertices_ += 6;

				// Matrix stack.
				// TODO: Factor into a seperate function.
				if( IsIdentity_ == BcFalse )
				{
					MaMat4d Matrix = getMatrix();

					for( BcU32 Idx = 0; Idx < 6; ++Idx )
					{
						ScnCanvasComponentVertex* pVertex = &pFirstVertex[ Idx ];
						MaVec3d Vertex = MaVec3d( pVertex->X_, pVertex->Y_, pVertex->Z_ ) * Matrix;
						pVertex->X_ = Vertex.x();
						pVertex->Y_ = Vertex.y();
						pVertex->Z_ = Vertex.z();
						pVertex->W_ = 1.0f;
					}
				}
				
				AddNewPrimitive = BcFalse;
			}
		}
		
		// Add primitive.
		if( AddNewPrimitive == BcTrue )
		{
			addPrimitive( RsTopologyType::TRIANGLE_LIST, pFirstVertex, 6, Layer, BcTrue );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// drawSprite
void ScnCanvasComponent::drawSprite3D( const MaVec3d& Position, const MaVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasComponentVertex* pVertices = allocVertices( 6 );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;
	
	const MaVec3d CornerA = Position;
	const MaVec3d CornerB = Position + MaVec3d( Size.x(), Size.y(), 0.0f );
	
	const ScnRect Rect = DiffuseTexture_.isValid() ? DiffuseTexture_->getRect( TextureIdx ) : ScnRect();
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 ABGR = Colour.asABGR();
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = Position.z();
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = Position.z();
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = Position.z();
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = Position.z();
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerA.y();
		pVertices->Z_ = Position.z();
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = CornerB.y();
		pVertices->Z_ = Position.z();
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->ABGR_ = ABGR;
		
		// Quickly check last primitive.
		BcBool AddNewPrimitive = BcTrue;
		if( LastPrimitiveSection_ != BcErrorCode )
		{
			ScnCanvasComponentPrimitiveSection& PrimitiveSection = PrimitiveSectionList_[ LastPrimitiveSection_ ];
			
			// If the last primitive was the same type as ours we can append to it.
			// NOTE: Need more checks here later.
			if( PrimitiveSection.Type_ == RsTopologyType::TRIANGLE_LIST &&
			   PrimitiveSection.Layer_ == Layer &&
			   PrimitiveSection.MaterialComponent_ == MaterialComponent_ )
			{
				PrimitiveSection.NoofVertices_ += 6;
				
				// Matrix stack.
				// TODO: Factor into a seperate function.
				if( IsIdentity_ == BcFalse )
				{
					MaMat4d Matrix = getMatrix();
					
					for( BcU32 Idx = 0; Idx < 6; ++Idx )
					{
						ScnCanvasComponentVertex* pVertex = &pFirstVertex[ Idx ];
						MaVec3d Vertex = MaVec3d( pVertex->X_, pVertex->Y_, pVertex->Z_ ) * Matrix;
						pVertex->X_ = Vertex.x();
						pVertex->Y_ = Vertex.y();
						pVertex->Z_ = Vertex.z();
						pVertex->W_ = 1.0f;
					}
				}
				
				AddNewPrimitive = BcFalse;
			}
		}
		
		// Add primitive.
		if( AddNewPrimitive == BcTrue )
		{
			addPrimitive( RsTopologyType::TRIANGLE_LIST, pFirstVertex, 6, Layer, BcTrue );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// drawSprite
void ScnCanvasComponent::drawSpriteUp3D( const MaVec3d& Position, const MaVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	ScnCanvasComponentVertex* pVertices = allocVertices( 6 );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;
	
	const MaVec3d CornerA = Position;
	const MaVec3d CornerB = Position + MaVec3d( Size.x(), 0.0f, Size.y() );
	
	const ScnRect Rect = DiffuseTexture_.isValid() ? DiffuseTexture_->getRect( TextureIdx ) : ScnRect();
	
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 ABGR = Colour.asABGR();
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = Position.y();
		pVertices->Z_ = CornerA.z();
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = Position.y();
		pVertices->Z_ = CornerA.z();
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = Position.y();
		pVertices->Z_ = CornerB.z();
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerA.x();
		pVertices->Y_ = Position.y();
		pVertices->Z_ = CornerB.z();
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = Position.y();
		pVertices->Z_ = CornerA.z();
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_;
		pVertices->ABGR_ = ABGR;
		++pVertices;
		
		pVertices->X_ = CornerB.x();
		pVertices->Y_ = Position.y();
		pVertices->Z_ = CornerB.z();
		pVertices->W_ = 1.0f;
		pVertices->U_ = Rect.X_ + Rect.W_;
		pVertices->V_ = Rect.Y_ + Rect.H_;
		pVertices->ABGR_ = ABGR;
		
		// Quickly check last primitive.
		BcBool AddNewPrimitive = BcTrue;
		if( LastPrimitiveSection_ != BcErrorCode )
		{
			ScnCanvasComponentPrimitiveSection& PrimitiveSection = PrimitiveSectionList_[ LastPrimitiveSection_ ];
			
			// If the last primitive was the same type as ours we can append to it.
			// NOTE: Need more checks here later.
			if( PrimitiveSection.Type_ == RsTopologyType::TRIANGLE_LIST &&
			   PrimitiveSection.Layer_ == Layer &&
			   PrimitiveSection.MaterialComponent_ == MaterialComponent_ )
			{
				PrimitiveSection.NoofVertices_ += 6;
				
				// Matrix stack.
				// TODO: Factor into a seperate function.
				if( IsIdentity_ == BcFalse )
				{
					MaMat4d Matrix = getMatrix();
					
					for( BcU32 Idx = 0; Idx < 6; ++Idx )
					{
						ScnCanvasComponentVertex* pVertex = &pFirstVertex[ Idx ];
						MaVec3d Vertex = MaVec3d( pVertex->X_, pVertex->Y_, pVertex->Z_ ) * Matrix;
						pVertex->X_ = Vertex.x();
						pVertex->Y_ = Vertex.y();
						pVertex->Z_ = Vertex.z();
						pVertex->W_ = 1.0f;
					}
				}
				
				AddNewPrimitive = BcFalse;
			}
		}
		
		// Add primitive.
		if( AddNewPrimitive == BcTrue )
		{
			addPrimitive( RsTopologyType::TRIANGLE_LIST, pFirstVertex, 6, Layer, BcTrue );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// drawSpriteCentered
void ScnCanvasComponent::drawSpriteCentered( const MaVec2d& Position, const MaVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	MaVec2d NewPosition = Position - ( Size * 0.5f );
	drawSprite( NewPosition, Size, TextureIdx, Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// drawSpriteCentered
void ScnCanvasComponent::drawSpriteCentered3D( const MaVec3d& Position, const MaVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	MaVec3d NewPosition = Position - MaVec3d( Size.x() * 0.5f, Size.y() * 0.5f, 0.0f );
	drawSprite3D( NewPosition, Size, TextureIdx, Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// drawSpriteCentered
void ScnCanvasComponent::drawSpriteCenteredUp3D( const MaVec3d& Position, const MaVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	MaVec3d NewPosition = Position - MaVec3d( Size.x() * 0.5f, 0.0f, Size.y() * 0.5f );
	drawSpriteUp3D( NewPosition, Size, TextureIdx, Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnCanvasComponent::clear()
{
	// Wait for vertex buffer to finish uploading.
	UploadFence_.wait();

	// Set vertices up.
	pVertices_ = pVerticesEnd_ = pWorkingVertices_;
	pVerticesEnd_ += NoofVertices_;
	VertexIndex_ = 0;
	
	// Empty primitive sections.
	PrimitiveSectionList_.clear();
	
	// Reset matrix stack.
	MatrixStack_.clear();
	MatrixStack_.push_back( MaMat4d() );
	IsIdentity_ = BcTrue;

	// Clear last primitive.
	LastPrimitiveSection_ = BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnCanvasComponent::preUpdate( BcF32 Tick )
{
	Super::update( Tick );

	// TODO: Clear in the pre-update tick.
	//clear();
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
			
			pContext_->setVertexBuffer( 0, VertexBuffer_, sizeof( ScnCanvasComponentVertex ) );
			pContext_->setVertexDeclaration( VertexDeclaration_ );
			pContext_->drawPrimitives( pPrimitiveSection->Type_, pPrimitiveSection->VertexIndex_, pPrimitiveSection->NoofVertices_ );
		}
	}
	
	BcU32 NoofSections_;
	ScnCanvasComponentPrimitiveSection* pPrimitiveSections_;
	RsBuffer* VertexBuffer_;
	RsVertexDeclaration* VertexDeclaration_;
};

void ScnCanvasComponent::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{
	// Upload.
	BcU32 VertexDataSize = VertexIndex_ * sizeof( ScnCanvasComponentVertex );
	if( VertexDataSize > 0 )
	{
		UploadFence_.increment();
		RsCore::pImpl()->updateBuffer( 
			RenderResource_.pVertexBuffer_, 0, VertexDataSize, 
			RsResourceUpdateFlags::ASYNC,
			[ this, VertexDataSize ]
			( RsBuffer* Buffer, const RsBufferLock& BufferLock )
			{
				BcAssert( VertexDataSize <= Buffer->getDesc().SizeBytes_ );
				BcMemCopy( BufferLock.Buffer_, pWorkingVertices_, 
					VertexDataSize );
				UploadFence_.decrement();
			} );
	}

	// HUD pass.
	Sort.Layer_ = 0;
	Sort.Pass_ = RS_SORT_PASS_MAX;

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
		pRenderNode->VertexBuffer_ = RenderResource_.pVertexBuffer_;
		pRenderNode->VertexDeclaration_ = VertexDeclaration_;
		
		// Copy primitive sections in.
		BcMemCopy( pRenderNode->pPrimitiveSections_, &PrimitiveSectionList_[ Idx ], sizeof( ScnCanvasComponentPrimitiveSection ) * 1 );
		
		// Bind material.
		// NOTE: We should be binding for every single draw call. We can have the material deal with redundancy internally
		//       if need be. If using multiple canvases we could potentially lose a material bind.
		//if( pLastMaterialComponent != pRenderNode->pPrimitiveSections_->MaterialComponent_ )
		{
			pLastMaterialComponent = pRenderNode->pPrimitiveSections_->MaterialComponent_;
			pLastMaterialComponent->bind( pFrame, Sort );
		}
		
		// Add to frame.
		pRenderNode->Sort_ = Sort;
		pFrame->addRenderNode( pRenderNode );
	}
	
	// Reset vertices.
	pVertices_ = pVerticesEnd_ = nullptr;
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
