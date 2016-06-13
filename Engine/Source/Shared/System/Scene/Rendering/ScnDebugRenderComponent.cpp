/**************************************************************************
*
* File:		ScnDebugRenderComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	13/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Rendering/ScnDebugRenderComponent.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"
#include "System/Content/CsPackage.h"

#include "Base/BcMath.h"

#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnDebugRenderComponent );

void ScnDebugRenderComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Material_",	&ScnDebugRenderComponent::Material_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "NoofVertices_", &ScnDebugRenderComponent::NoofVertices_, bcRFF_IMPORTER | bcRFF_CONST ),

		new ReField( "MaterialComponent_",	&ScnDebugRenderComponent::MaterialComponent_, bcRFF_TRANSIENT ),
		new ReField( "VertexIndex_", &ScnDebugRenderComponent::VertexIndex_ ),
	};
		
	using namespace std::placeholders;
	ReRegisterClass< ScnDebugRenderComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( 
			{
				ScnComponentProcessFuncEntry(
					"Simulate",
					ScnComponentPriority::DEBUG_RENDER_CLEAR,
					std::bind( &ScnDebugRenderComponent::clearAll, _1 ) )
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Statics
ScnDebugRenderComponent* ScnDebugRenderComponent::pImpl_ = NULL;

//static
ScnDebugRenderComponent* ScnDebugRenderComponent::pImpl()
{
	return pImpl_;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnDebugRenderComponent::ScnDebugRenderComponent():
	ScnDebugRenderComponent( 0 )
{
	setPasses( RsRenderSortPassFlags::TRANSPARENT );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnDebugRenderComponent::ScnDebugRenderComponent( BcU32 NoofVertices ):
	VertexDeclaration_( nullptr ),
	pVertices_( nullptr ),
	pVerticesEnd_( nullptr ),
	pWorkingVertices_( nullptr ),
	NoofVertices_( NoofVertices )
{
	setPasses( RsRenderSortPassFlags::TRANSPARENT );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnDebugRenderComponent::~ScnDebugRenderComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
MaAABB ScnDebugRenderComponent::getAABB() const
{
	return MaAABB();
}

//////////////////////////////////////////////////////////////////////////
// allocVertices
ScnDebugRenderComponentVertex* ScnDebugRenderComponent::allocVertices( BcU32 NoofVertices )
{
	BcAssertMsg( pVertices_ != nullptr, "ScnDebugRenderComponent: Don't have a working buffer." );
	ScnDebugRenderComponentVertex* pCurrVertex = NULL;
	if( ( VertexIndex_ + NoofVertices ) <= NoofVertices_ )
	{
		pCurrVertex = &pVertices_[ VertexIndex_ ];
		VertexIndex_ += NoofVertices;
	}
	return pCurrVertex;
}

//////////////////////////////////////////////////////////////////////////
// addPrimitive
void ScnDebugRenderComponent::addPrimitive( RsTopologyType Type, ScnDebugRenderComponentVertex* pVertices, BcU32 NoofVertices, BcU32 Layer, BcBool UseMatrixStack )
{
	BcAssertMsg( MaterialComponent_ != nullptr, "ScnDebugRenderComponent: Material component has not been set!" );

	// Check if the vertices are owned by us, if not copy in.
	if( pVertices < pVertices_ || pVertices_ >= pVerticesEnd_ )
	{
		ScnDebugRenderComponentVertex* pNewVertices = allocVertices( NoofVertices );
		if( pNewVertices != NULL )
		{
			BcMemCopy( pNewVertices, pVertices, sizeof( ScnDebugRenderComponentVertex ) * NoofVertices );
			pVertices = pNewVertices;
		}
	}
	
	// TODO: If there was a previous primitive which we can marge into, attempt to.
	BcU32 VertexIndex = convertVertexPointerToIndex( pVertices );
	ScnDebugRenderComponentPrimitiveSection PrimitiveSection = 
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
void ScnDebugRenderComponent::drawLine( const MaVec3d& PointA, const MaVec3d& PointB, const RsColour& Colour, BcU32 Layer )
{
	ScnDebugRenderComponentVertex* pVertices = allocVertices( 2 );
	ScnDebugRenderComponentVertex* pFirstVertex = pVertices;
	
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

		// Quickly check last primitive.
		BcBool AddNewPrimitive = BcTrue;
		if( LastPrimitiveSection_ != BcErrorCode )
		{
			ScnDebugRenderComponentPrimitiveSection& PrimitiveSection = PrimitiveSectionList_[ LastPrimitiveSection_ ];

			// If the last primitive was the same type as ours we can append to it.
			// NOTE: Need more checks here later.
			if( PrimitiveSection.Type_ == RsTopologyType::LINE_LIST &&
				PrimitiveSection.Layer_ == Layer &&
				PrimitiveSection.MaterialComponent_ == MaterialComponent_ )
			{
				PrimitiveSection.NoofVertices_ += 2;
				
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
void ScnDebugRenderComponent::drawLines( const MaVec3d* pPoints, BcU32 NoofLines, const RsColour& Colour, BcU32 Layer )
{
	BcU32 NoofVertices = 2 * NoofLines;
	ScnDebugRenderComponentVertex* pVertices = allocVertices( NoofVertices );
	ScnDebugRenderComponentVertex* pFirstVertex = pVertices;

	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{	
		// Now copy in data.
		BcU32 ABGR = Colour.asABGR();

		for( BcU32 Idx = 0; Idx < NoofVertices; ++Idx )
		{
			pVertices->X_ = pPoints[ Idx ].x();
			pVertices->Y_ = pPoints[ Idx ].y();
			pVertices->Z_ = pPoints[ Idx ].z();
			pVertices->W_ = 1.0f;
			pVertices->ABGR_ = ABGR;
			++pVertices;
		}
		
		// Add primitive.		
		addPrimitive( RsTopologyType::LINE_STRIP, pFirstVertex, NoofVertices, Layer, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// drawMatrix
void ScnDebugRenderComponent::drawMatrix( const MaMat4d& Matrix, const RsColour& Colour, BcU32 Layer )
{
	MaVec3d Centre( Matrix.translation() );	
	MaVec3d X( Matrix.row0().x(), Matrix.row0().y(), Matrix.row0().z() );	
	MaVec3d Y( Matrix.row1().x(), Matrix.row1().y(), Matrix.row1().z() );	
	MaVec3d Z( Matrix.row2().x(), Matrix.row2().y(), Matrix.row2().z() );	

	drawLine( Centre, Centre + X, RsColour::RED * Colour, Layer );
	drawLine( Centre, Centre + Y, RsColour::GREEN * Colour, Layer );
	drawLine( Centre, Centre + Z, RsColour::BLUE * Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// drawGrid
void ScnDebugRenderComponent::drawGrid( const MaVec3d& Position, const MaVec3d& Size, BcF32 StepSize, BcF32 SubDivideMultiple, BcU32 Layer )
{
	BcU32 NoofAxis = ( Size.x() > 0 ? 1 : 0  ) + ( Size.y() > 0 ? 1 : 0  ) + ( Size.z() > 0 ? 1 : 0 );
	BcAssertMsg( NoofAxis == 2, "Only supports 2 axis in the grid!" );
	BcAssert( SubDivideMultiple > 1.0f );
	BcUnusedVar( NoofAxis );
	
	
	// Determine which axis to draw along.
	MaVec3d XAxis;
	MaVec3d YAxis;
	BcF32 XSize = 0.0f;
	BcF32 YSize = 0.0f;

	if( Size.x() > 0.0f )
	{
		XAxis = MaVec3d( Size.x(), 0.0f, 0.0f );
		XSize = Size.x();
		if( Size.y() > 0.0f )
		{
			YAxis = MaVec3d( 0.0f, Size.y(), 0.0f );
			YSize = Size.y();
		}
		else
		{
			YAxis = MaVec3d( 0.0f, 0.0f, Size.z() );
			YSize = Size.z();
		}
	}
	else
	{
		XAxis = MaVec3d( 0.0f, Size.y(), 0.0f );
		YAxis = MaVec3d( 0.0f, 0.0f, Size.z() );
		XSize = Size.y();
		YSize = Size.z();
	}

	// Normalise.
	XAxis.normalise();
	YAxis.normalise();

	while( StepSize < XSize && StepSize < YSize )
	{
		// Draw grid.
		for( BcF32 X = 0.0f; X <= XSize; X += StepSize )
		{
			MaVec3d A1( Position + ( XAxis * X ) + ( YAxis *  YSize ) );
			MaVec3d B1( Position + ( XAxis * X ) + ( YAxis * -YSize ) );
			MaVec3d A2( Position + ( XAxis * -X ) + ( YAxis *  YSize ) );
			MaVec3d B2( Position + ( XAxis * -X ) + ( YAxis * -YSize ) );
			drawLine( A1, B1, RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), Layer );
			drawLine( A2, B2, RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), Layer );
		}

		for( BcF32 Y = 0.0f; Y <= YSize; Y += StepSize )
		{
			MaVec3d A1( Position + ( XAxis *  YSize ) + ( YAxis * Y ) );
			MaVec3d B1( Position + ( XAxis * -YSize ) + ( YAxis * Y ) );
			MaVec3d A2( Position + ( XAxis *  YSize ) + ( YAxis * -Y ) );
			MaVec3d B2( Position + ( XAxis * -YSize ) + ( YAxis * -Y ) );
			drawLine( A1, B1, RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), Layer );
			drawLine( A2, B2, RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), Layer );
		}

		StepSize *= SubDivideMultiple;
	}
}

//////////////////////////////////////////////////////////////////////////
// drawEllipsoid
void ScnDebugRenderComponent::drawEllipsoid( const MaVec3d& Position, const MaVec3d& Size, const RsColour& Colour, BcU32 Layer )
{
	// Draw outer circles for all axis.
	BcU32 LOD = 16;
	BcF32 Angle = 0.0f;
	BcF32 AngleInc = ( BcPI * 2.0f ) / BcF32( LOD );

	// Draw axis lines.
	for( BcU32 i = 0; i < LOD; ++i )
	{
		MaVec2d PosA( BcCos( Angle ), -BcSin( Angle ) );
		MaVec2d PosB( BcCos( Angle + AngleInc ), -BcSin( Angle + AngleInc ) );

		MaVec3d XAxisA = MaVec3d( 0.0f,                 PosA.x() * Size.y(), PosA.y() * Size.z() );
		MaVec3d YAxisA = MaVec3d( PosA.x() * Size.x(), 0.0f,                 PosA.y() * Size.z() );
		MaVec3d ZAxisA = MaVec3d( PosA.x() * Size.x(), PosA.y() * Size.y(), 0.0f                 );
		MaVec3d XAxisB = MaVec3d( 0.0f,                 PosB.x() * Size.y(), PosB.y() * Size.z() );
		MaVec3d YAxisB = MaVec3d( PosB.x() * Size.x(), 0.0f,                 PosB.y() * Size.z() );
		MaVec3d ZAxisB = MaVec3d( PosB.x() * Size.x(), PosB.y() * Size.y(), 0.0f                 );

		drawLine( XAxisA + Position, XAxisB + Position, Colour, 0 );
		drawLine( YAxisA + Position, YAxisB + Position, Colour, 0 );
		drawLine( ZAxisA + Position, ZAxisB + Position, Colour, 0 );

		Angle += AngleInc;
	}

	// Draw a cross down centre.
	MaVec3d XAxis = MaVec3d( Size.x(), 0.0f, 0.0f );
	MaVec3d YAxis = MaVec3d( 0.0f, Size.y(), 0.0f );
	MaVec3d ZAxis = MaVec3d( 0.0f, 0.0f, Size.z() );
	drawLine( Position - XAxis, Position + XAxis, Colour, Layer );
	drawLine( Position - YAxis, Position + YAxis, Colour, Layer );
	drawLine( Position - ZAxis, Position + ZAxis, Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// drawCircle
void ScnDebugRenderComponent::drawCircle( const MaVec3d& Position, const MaVec3d& Size, const RsColour& Colour, BcU32 Layer )
{
	// Draw outer circles for all axis.
	BcU32 LOD = 12;
	BcF32 Angle = 0.0f;
	BcF32 AngleInc = ( BcPI * 2.0f ) / BcF32( LOD );

	// Draw axis lines.
	for( BcU32 i = 0; i < LOD; ++i )
	{
		MaVec2d PosA( BcCos( Angle ), -BcSin( Angle ) );
		MaVec2d PosB( BcCos( Angle + AngleInc ), -BcSin( Angle + AngleInc ) );

		//MaVec3d XAxisA = MaVec3d( 0.0f,                 PosA.x() * Size.y(), PosA.y() * Size.z() );
		MaVec3d YAxisA = MaVec3d( PosA.x() * Size.x(), 0.0f,                 PosA.y() * Size.z() );
		//MaVec3d ZAxisA = MaVec3d( PosA.x() * Size.x(), PosA.y() * Size.y(), 0.0f                 );
		//MaVec3d XAxisB = MaVec3d( 0.0f,                 PosB.x() * Size.y(), PosB.y() * Size.z() );
		MaVec3d YAxisB = MaVec3d( PosB.x() * Size.x(), 0.0f,                 PosB.y() * Size.z() );
		//MaVec3d ZAxisB = MaVec3d( PosB.x() * Size.x(), PosB.y() * Size.y(), 0.0f                 );

		//drawLine( XAxisA + Position, XAxisB + Position, Colour, 0 );
		drawLine( YAxisA + Position, YAxisB + Position, Colour, 0 );
		//drawLine( ZAxisA + Position, ZAxisB + Position, Colour, 0 );

		Angle += AngleInc;
	}

	// Draw a cross down centre.
	MaVec3d XAxis = MaVec3d( Size.x(), 0.0f, 0.0f );
	MaVec3d YAxis = MaVec3d( 0.0f, Size.y(), 0.0f );
	MaVec3d ZAxis = MaVec3d( 0.0f, 0.0f, Size.z() );
	drawLine( Position - XAxis, Position + XAxis, Colour, Layer );
	drawLine( Position - YAxis, Position + YAxis, Colour, Layer );
	drawLine( Position - ZAxis, Position + ZAxis, Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// drawAABB
void ScnDebugRenderComponent::drawAABB( const MaAABB& AABB, const RsColour& Colour, BcU32 Layer )
{
	drawLine( AABB.corner( 0 ), AABB.corner( 1 ), Colour, Layer );
	drawLine( AABB.corner( 1 ), AABB.corner( 3 ), Colour, Layer );
	drawLine( AABB.corner( 2 ), AABB.corner( 0 ), Colour, Layer );
	drawLine( AABB.corner( 3 ), AABB.corner( 2 ), Colour, Layer );

	drawLine( AABB.corner( 4 ), AABB.corner( 5 ), Colour, Layer );
	drawLine( AABB.corner( 5 ), AABB.corner( 7 ), Colour, Layer );
	drawLine( AABB.corner( 6 ), AABB.corner( 4 ), Colour, Layer );
	drawLine( AABB.corner( 7 ), AABB.corner( 6 ), Colour, Layer );

	drawLine( AABB.corner( 0 ), AABB.corner( 4 ), Colour, Layer );
	drawLine( AABB.corner( 1 ), AABB.corner( 5 ), Colour, Layer );
	drawLine( AABB.corner( 2 ), AABB.corner( 6 ), Colour, Layer );
	drawLine( AABB.corner( 3 ), AABB.corner( 7 ), Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnDebugRenderComponent::clear()
{
	// Wait for upload to have completed so we can use vertices.
	UploadFence_.wait();

	// Set vertices up.
	pVertices_ = pVerticesEnd_ = pWorkingVertices_;
	pVerticesEnd_ += NoofVertices_;
	VertexIndex_ = 0;
	
	// Empty primitive sections.
	PrimitiveSectionList_.clear();
	
	// Clear last primitive.
	LastPrimitiveSection_ = BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnDebugRenderComponent::render( ScnRenderContext& RenderContext )
{
	// Upload.
	BcU32 VertexDataSize = VertexIndex_ * sizeof( ScnDebugRenderComponentVertex );
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
	Sort.Layer_ = RS_SORT_LAYER_MAX;

	// NOTE: Could do this sort inside of the renderer, but I'm just gonna keep the canvas
	//       as one solid object as to not conflict with other canvas objects when rendered
	//       to the scene. Will not sort by transparency or anything either.
	std::stable_sort( PrimitiveSectionList_.begin(), PrimitiveSectionList_.end(), ScnDebugRenderComponentPrimitiveSectionCompare() );
	
	// Cache last material instance.
	ScnMaterialComponent* pLastMaterialComponent = NULL;
	
	for( BcU32 Idx = 0; Idx < PrimitiveSectionList_.size(); ++Idx )
	{
		auto* PrimitiveSection = &PrimitiveSectionList_[ Idx ];

		// Bind material.
		// NOTE: We should be binding for every single draw call. We can have the material deal with redundancy internally
		//       if need be. If using multiple canvases we could potentially lose a material bind.
		//if( pLastMaterialComponent != pRenderNode->pPrimitiveSections_->MaterialComponent_ )
		{
			pLastMaterialComponent = PrimitiveSection->MaterialComponent_;

			// Set model parameters on material.
			ObjectUniforms_.WorldTransform_ = getParentEntity()->getWorldMatrix();
			RsCore::pImpl()->updateBuffer( 
				UniformBuffer_.get(),
				0, sizeof( ObjectUniforms_ ),
				RsResourceUpdateFlags::ASYNC,
				[ 
					ObjectUniforms = ObjectUniforms_
				]
				( RsBuffer* Buffer, const RsBufferLock& Lock )
				{
					BcMemCopy( Lock.Buffer_, &ObjectUniforms, sizeof( ObjectUniforms ) );					
				} );
			pLastMaterialComponent->setObjectUniformBlock( UniformBuffer_.get() );

			RenderContext.pViewComponent_->setMaterialParameters( pLastMaterialComponent );
		}
		
		// Add to frame.
		RenderContext.pFrame_->queueRenderNode( Sort,
			[ 
				GeometryBinding = GeometryBinding_.get(),
				ProgramBinding = MaterialComponent_->getProgramBinding(),
				RenderState = MaterialComponent_->getRenderState(),
				FrameBuffer = RenderContext.pViewComponent_->getFrameBuffer(),
				Viewport = RenderContext.pViewComponent_->getViewport(),
				PrimitiveSection = *PrimitiveSection
			]
			( RsContext* Context )
			{
				Context->drawPrimitives( 
					GeometryBinding,
					ProgramBinding,
					RenderState,
					FrameBuffer,
					&Viewport,
					nullptr,
					PrimitiveSection.Type_, PrimitiveSection.VertexIndex_, PrimitiveSection.NoofVertices_,
					0, 1 );
			} );
	}
	
	// Reset vertex index.
	VertexIndex_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnDebugRenderComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	BcAssert( Material_ != nullptr );
	BcAssert( NoofVertices_ > 0 );

	MaterialComponent_ = Parent->attach< ScnMaterialComponent >( 
		BcName::INVALID,
		Material_,
		ScnShaderPermutationFlags::MESH_STATIC_3D | ScnShaderPermutationFlags::LIGHTING_NONE );

	// Allocate our own vertex buffer data.
	VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration( 
		RsVertexDeclarationDesc( 2 )
			.addElement( RsVertexElement( 0, 0,				4,		RsVertexDataType::FLOAT32,		RsVertexUsage::POSITION,		0 ) )
			.addElement( RsVertexElement( 0, 16,			4,		RsVertexDataType::UBYTE_NORM,	RsVertexUsage::COLOUR,			0 ) ),
		getFullName().c_str() );
	
	// Allocate render side vertex buffer.
	VertexBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc( 
			RsResourceBindFlags::VERTEX_BUFFER,
			RsResourceCreationFlags::STREAM,
			NoofVertices_ * sizeof( ScnDebugRenderComponentVertex ) ),
		getFullName().c_str() );

	RsGeometryBindingDesc GeometryBindingDesc;
	GeometryBindingDesc.setVertexDeclaration( VertexDeclaration_.get() );
	GeometryBindingDesc.setVertexBuffer( 0, VertexBuffer_.get(), sizeof( ScnDebugRenderComponentVertex ) );
	GeometryBinding_ = RsCore::pImpl()->createGeometryBinding( GeometryBindingDesc, getFullName().c_str() );

	// Allocate uniform buffer object.
	UniformBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc( 
			RsResourceBindFlags::UNIFORM_BUFFER,
			RsResourceCreationFlags::STREAM,
			sizeof( ScnShaderObjectUniformBlockData ) ),
		getFullName().c_str() );

	// Allocate working vertices.
	pWorkingVertices_ = new ScnDebugRenderComponentVertex[ NoofVertices_ ];

	BcAssert( pImpl_ == NULL );
	pImpl_ = this;
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnDebugRenderComponent::onDetach( ScnEntityWeakRef Parent )
{
	getParentEntity()->detach( MaterialComponent_ );

	UploadFence_.wait();

	// Delete working data.
	delete [] pWorkingVertices_;

	BcAssert( pImpl_ == this );
	pImpl_ = NULL;

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// clearAll
//static
void ScnDebugRenderComponent::clearAll( const ScnComponentList& Components )
{
	for( auto Component : Components )
	{
		BcAssert( Component->isTypeOf< ScnDebugRenderComponent >() );
		auto* DebugRenderComponent = static_cast< ScnDebugRenderComponent* >( Component.get() );

		DebugRenderComponent->clear();
	}
}