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
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"
#include "System/Os/OsCore.h"
#include "System/Os/OsClient.h"

#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnCanvasComponent );

void ScnCanvasComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "NoofVertices_", &ScnCanvasComponent::NoofVertices_, bcRFF_IMPORTER | bcRFF_CONST ),
		new ReField( "Clear_", &ScnCanvasComponent::Clear_, bcRFF_IMPORTER ),
		new ReField( "AbsoluteCoords_", &ScnCanvasComponent::AbsoluteCoords_, bcRFF_IMPORTER ),
		new ReField( "Left_", &ScnCanvasComponent::Left_, bcRFF_IMPORTER ),
		new ReField( "Right_", &ScnCanvasComponent::Right_, bcRFF_IMPORTER ),
		new ReField( "Top_", &ScnCanvasComponent::Top_, bcRFF_IMPORTER ),
		new ReField( "Bottom_", &ScnCanvasComponent::Bottom_, bcRFF_IMPORTER ),

		new ReField( "HaveVertexBufferLock_", &ScnCanvasComponent::HaveVertexBufferLock_, bcRFF_TRANSIENT ),
		new ReField( "VertexIndex_", &ScnCanvasComponent::VertexIndex_, bcRFF_TRANSIENT ),
		new ReField( "ViewMatrix_", &ScnCanvasComponent::ViewMatrix_ ),
		new ReField( "MaterialComponent_", &ScnCanvasComponent::MaterialComponent_ ),
		new ReField( "DiffuseTexture_", &ScnCanvasComponent::DiffuseTexture_, bcRFF_TRANSIENT ),
		new ReField( "MatrixStack_", &ScnCanvasComponent::MatrixStack_ ),
		new ReField( "IsIdentity_", &ScnCanvasComponent::IsIdentity_ ),
	};

	using namespace std::placeholders;		
	ReRegisterClass< ScnCanvasComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( 
			{
				ScnComponentProcessFuncEntry(
					"Clear",
					ScnComponentPriority::DEBUG_RENDER_CLEAR,
					std::bind( &ScnCanvasComponent::clearAll, _1 ) ),
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnCanvasComponent::ScnCanvasComponent():
	ScnCanvasComponent( 0 )
{
	setPasses( RsRenderSortPassFlags::OVERLAY );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnCanvasComponent::ScnCanvasComponent( BcU32 NoofVertices ):
	HaveVertexBufferLock_( BcFalse ),
	IsIdentity_( BcTrue ),
	pVertices_( nullptr ),
	pVerticesEnd_( nullptr ),
	pWorkingVertices_( nullptr ),
	VertexIndex_ ( 0 ),
	VertexDeclaration_( nullptr ),
	NoofVertices_( NoofVertices ),
	Clear_( BcFalse ),
	Left_( 0.0f ),
	Right_( 0.0f ),
	Top_( 0.0f ),
	Bottom_( 0.0f )
{
	setPasses( RsRenderSortPassFlags::OVERLAY );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnCanvasComponent::~ScnCanvasComponent()
{

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
MaMat4d ScnCanvasComponent::popMatrix()
{
	BcAssertMsg( MatrixStack_.size(), "ScnCanvasComponent: Can't pop the last matrix off the stack! Mismatching push/pop?" );
	auto RetVal = getMatrix();
	if( MatrixStack_.size() > 1 )
	{
		MatrixStack_.pop_back();
		const MaMat4d& CurrMatrix = getMatrix();
		IsIdentity_ = CurrMatrix.isIdentity();
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// setMatrix
void ScnCanvasComponent::setMatrix( const MaMat4d& Matrix )
{
	MatrixStack_[ MatrixStack_.size() - 1 ] = Matrix;
}

MaMat4d ScnCanvasComponent::getMatrix() const
{
	return MatrixStack_[ MatrixStack_.size() - 1 ];
}

//////////////////////////////////////////////////////////////////////////
// setViewMatrix
void ScnCanvasComponent::setViewMatrix( const MaMat4d& View )
{
	ViewMatrix_ = View;
}

//////////////////////////////////////////////////////////////////////////
// getRect
const ScnRect& ScnCanvasComponent::getRect( BcU32 Idx ) const
{
	static ScnRect Rect = 
	{
		0.0f, 0.0f,
		1.0f, 1.0f
	};
	return DiffuseTexture_.isValid() ? DiffuseTexture_->getRect( Idx ) : Rect;
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
// addCustomRender
void ScnCanvasComponent::addCustomRender( 
	std::function< void( class RsContext* ) > CustomRenderFunc,
	BcU32 Layer )
{
	ScnCanvasComponentPrimitiveSection PrimitiveSection = 
	{
		RsTopologyType::INVALID,
		0,
		0,
		Layer,
		MaterialComponent_,
		CustomRenderFunc
	};
	
	PrimitiveSectionList_.push_back( PrimitiveSection );
	LastPrimitiveSection_ = (BcU32)PrimitiveSectionList_.size() - 1;
}

//////////////////////////////////////////////////////////////////////////
// addPrimitive
void ScnCanvasComponent::addPrimitive( 
		RsTopologyType Type, 
		ScnCanvasComponentVertex* pVertices, 
		BcU32 NoofVertices, 
		BcU32 Layer, 
		BcBool UseMatrixStack )
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
		MaterialComponent_,
		nullptr
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

	const ScnRect& Rect = getRect( TextureIdx );

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
// drawSpriteCentered
void ScnCanvasComponent::drawSpriteCentered( const MaVec2d& Position, const MaVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer )
{
	MaVec2d NewPosition = Position - ( Size * 0.5f );
	drawSprite( NewPosition, Size, TextureIdx, Colour, Layer );
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
// render
void ScnCanvasComponent::render( ScnRenderContext & RenderContext )
{
	// Upload.
	size_t VertexDataSize = VertexIndex_ * sizeof( ScnCanvasComponentVertex );
	if( VertexDataSize > 0 )
	{
		UploadFence_.increment();
		RsCore::pImpl()->updateBuffer( 
			VertexBuffer_.get(), 0, VertexDataSize, 
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
	RsRenderSort Sort = RenderContext.Sort_;
	Sort.Layer_ = 0;
	Sort.Pass_ = RS_SORT_PASS_MAX;

	// NOTE: Could do this sort inside of the renderer, but I'm just gonna keep the canvas
	//       as one solid object as to not conflict with other canvas objects when rendered
	//       to the scene. Will not sort by transparency or anything either.
	//std::stable_sort( PrimitiveSectionList_.begin(), PrimitiveSectionList_.end(), ScnCanvasComponentPrimitiveSectionCompare() );
	
	for( BcU32 Idx = 0; Idx < PrimitiveSectionList_.size(); ++Idx )
	{
		// Copy primitive sections in.
		auto* PrimitiveSection = RenderContext.pFrame_->alloc< ScnCanvasComponentPrimitiveSection >( 1 );
		BcMemZero( PrimitiveSection, sizeof( ScnCanvasComponentPrimitiveSection ) );
		*PrimitiveSection = PrimitiveSectionList_[ Idx ];
		
		// Add to frame.
		UploadFence_.increment();
		RenderContext.pFrame_->queueRenderNode( Sort,
			[
				this,
				GeometryBinding = GeometryBinding_.get(),
				ProgramBinding = PrimitiveSection->MaterialComponent_->getProgramBinding(),
				RenderState = PrimitiveSection->MaterialComponent_->getRenderState(),
				FrameBuffer = RenderContext.pViewComponent_->getFrameBuffer(),
				Viewport = RenderContext.pViewComponent_->getViewport(),
				PrimitiveSection 
			]
			( RsContext* Context )
			{
				if( PrimitiveSection->RenderFunc_ != nullptr )
				{
					PrimitiveSection->RenderFunc_( Context );
				}

				if( PrimitiveSection->Type_ != RsTopologyType::INVALID )
				{
					Context->drawPrimitives( 
						GeometryBinding,
						ProgramBinding,
						RenderState,
						FrameBuffer,
						&Viewport,
						nullptr,
						PrimitiveSection->Type_, PrimitiveSection->VertexIndex_, PrimitiveSection->NoofVertices_ );
				}

				PrimitiveSection->~ScnCanvasComponentPrimitiveSection();
				UploadFence_.decrement();
			} );
	}
	
	// Reset vertices.
	pVertices_ = pVerticesEnd_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnCanvasComponent::onAttach( ScnEntityWeakRef Parent )
{
	BcAssert( NoofVertices_ > 0 );

	// Allocate our own vertex buffer data.
	VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration( 
		RsVertexDeclarationDesc( 3 )
			.addElement( RsVertexElement( 0, 0,				4,		RsVertexDataType::FLOAT32,		RsVertexUsage::POSITION,		0 ) )
			.addElement( RsVertexElement( 0, 16,			2,		RsVertexDataType::FLOAT32,		RsVertexUsage::TEXCOORD,		0 ) )
			.addElement( RsVertexElement( 0, 24,			4,		RsVertexDataType::UBYTE_NORM,	RsVertexUsage::COLOUR,			0 ) ),
		getFullName().c_str() );

	// Allocate render resources.
	VertexBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc( 
			RsBufferType::VERTEX,
			RsResourceCreationFlags::STREAM,
			NoofVertices_ * sizeof( ScnCanvasComponentVertex ) ),
		getFullName().c_str() );

	RsGeometryBindingDesc GeometryBindingDesc;
	GeometryBindingDesc.setVertexDeclaration( VertexDeclaration_.get() );
	GeometryBindingDesc.setVertexBuffer( 0, VertexBuffer_.get(), sizeof( ScnCanvasComponentVertex ) );
	GeometryBinding_ = RsCore::pImpl()->createGeometryBinding( GeometryBindingDesc, getFullName().c_str() );

	// Allocate working vertices.
	pWorkingVertices_ = new ScnCanvasComponentVertex[ NoofVertices_ ];

	Super::onAttach( Parent );	
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnCanvasComponent::onDetach( ScnEntityWeakRef Parent )
{
	UploadFence_.wait();

	// Delete working data.
	delete [] pWorkingVertices_;

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// clearAll
//virtual
void ScnCanvasComponent::clearAll( const ScnComponentList& Components )
{
	for( auto Component : Components )
	{
		BcAssert( Component->isTypeOf< ScnCanvasComponent >() );
		auto* CanvasComponent = static_cast< ScnCanvasComponent* >( Component.get() );

		if( CanvasComponent->Clear_ )
		{
			CanvasComponent->clear();

			// Push new ortho matrix.
			// Just use default client size.
			auto Client = OsCore::pImpl()->getClient( 0 );

			const BcF32 Width = CanvasComponent->AbsoluteCoords_ ? 1.0f : Client->getWidth();
			const BcF32 Height = CanvasComponent->AbsoluteCoords_ ? 1.0f : Client->getHeight();

			MaMat4d Projection;
			Projection.orthoProjection(
				CanvasComponent->Left_ * Width,
				CanvasComponent->Right_ * Width,
				CanvasComponent->Bottom_ * Height,
				CanvasComponent->Top_ * Height,
				-1.0f, 
				1.0f );

			// Push projection matrix onto stack.
			CanvasComponent->pushMatrix( Projection );

			// Push view matrix onto stack.
			CanvasComponent->pushMatrix( CanvasComponent->ViewMatrix_ );
		}
	}
}
