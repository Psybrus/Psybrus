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

#include "System/Scene/ScnDebugRenderComponent.h"
#include "System/Scene/ScnEntity.h"
#include "System/Scene/ScnViewComponent.h"

#include "System/Content/CsCore.h"
#include "System/Content/CsPackage.h"

#ifdef PSY_SERVER
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnDebugRenderComponent );

BCREFLECTION_EMPTY_REGISTER( ScnDebugRenderComponent );
/*
BCREFLECTION_DERIVED_BEGIN( ScnComponent, ScnDebugRenderComponent )
	BCREFLECTION_MEMBER( BcName,							Name_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								Index_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( CsPackage,							pPackage_,						bcRFF_POINTER | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								RefCount_,						bcRFF_DEFAULT | bcRFF_TRANSIENT ),
BCREFLECTION_DERIVED_END();
*/

//////////////////////////////////////////////////////////////////////////
// Statics
ScnDebugRenderComponent* ScnDebugRenderComponent::pImpl_ = NULL;

//static
ScnDebugRenderComponent* ScnDebugRenderComponent::pImpl()
{
	BcAssert( pImpl_ != NULL );
	return pImpl_;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnDebugRenderComponent::initialise( BcU32 NoofVertices )
{
	Super::initialise();

	// NULL internals.
	BcMemZero( &RenderResources_[ 0 ], sizeof( RenderResources_ ) );
	HaveVertexBufferLock_ = BcFalse;
	
	// Store number of vertices.
	NoofVertices_ = NoofVertices;
	
	// Which render resource to use.
	CurrentRenderResource_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnDebugRenderComponent::initialise( const Json::Value& Object )
{
	ScnDebugRenderComponent::initialise( Object[ "noofvertices" ].asUInt() );

	ScnMaterialRef Material = getPackage()->getPackageCrossRef( Object[ "material" ].asUInt() );

	CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), MaterialComponent_, Material, scnSPF_STATIC_3D | scnSPF_UNLIT );
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnDebugRenderComponent::create()
{
	// Allocate our own vertex buffer data.
	BcU32 VertexFormat = rsVDF_POSITION_XYZ | rsVDF_COLOUR_ABGR8;
	BcAssert( RsVertexDeclSize( VertexFormat ) == sizeof( ScnDebugRenderComponentVertex ) );

	// Allocate render resources.
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TRenderResource& RenderResource = RenderResources_[ Idx ];

		// Allocate vertices.
		RenderResource.pVertices_ = new ScnDebugRenderComponentVertex[ NoofVertices_ ];

		// Allocate render side vertex buffer.
		RenderResource.pVertexBuffer_ = RsCore::pImpl()->createVertexBuffer( VertexFormat, NoofVertices_, RenderResource.pVertices_ );
	
		// Allocate render side primitive.
		RenderResource.pPrimitive_ = RsCore::pImpl()->createPrimitive( RenderResource.pVertexBuffer_, NULL );
	}

	Super::create();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnDebugRenderComponent::destroy()
{
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TRenderResource& RenderResource = RenderResources_[ Idx ];

		// Allocate render side vertex buffer.
		RsCore::pImpl()->destroyResource( RenderResource.pVertexBuffer_ );
	
		// Allocate render side primitive.
		RsCore::pImpl()->destroyResource( RenderResource.pPrimitive_ );
	}

	// Wait for renderer.
	SysFence Fence( RsCore::WORKER_MASK );

	// Delete working data.
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TRenderResource& RenderResource = RenderResources_[ Idx ];

		// Delete vertices.
		delete [] RenderResource.pVertices_;
	}
}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
BcAABB ScnDebugRenderComponent::getAABB() const
{
	return BcAABB();
}

//////////////////////////////////////////////////////////////////////////
// allocVertices
ScnDebugRenderComponentVertex* ScnDebugRenderComponent::allocVertices( BcU32 NoofVertices )
{
	BcAssertMsg( HaveVertexBufferLock_ == BcTrue, "ScnDebugRenderComponent: Don't have vertex buffer lock!" );
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
void ScnDebugRenderComponent::addPrimitive( eRsPrimitiveType Type, ScnDebugRenderComponentVertex* pVertices, BcU32 NoofVertices, BcU32 Layer, BcBool UseMatrixStack )
{
	BcAssertMsg( MaterialComponent_.isValid(), "ScnDebugRenderComponent: Material component has not been set!" );

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
	LastPrimitiveSection_ = PrimitiveSectionList_.size() - 1;
}

//////////////////////////////////////////////////////////////////////////
// drawLine
void ScnDebugRenderComponent::drawLine( const BcVec3d& PointA, const BcVec3d& PointB, const RsColour& Colour, BcU32 Layer )
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
		pVertices->ABGR_ = ABGR;
		++pVertices;
		pVertices->X_ = PointB.x();
		pVertices->Y_ = PointB.y();
		pVertices->Z_ = PointB.z();
		pVertices->ABGR_ = ABGR;

		// Quickly check last primitive.
		BcBool AddNewPrimitive = BcTrue;
		if( LastPrimitiveSection_ != BcErrorCode )
		{
			ScnDebugRenderComponentPrimitiveSection& PrimitiveSection = PrimitiveSectionList_[ LastPrimitiveSection_ ];

			// If the last primitive was the same type as ours we can append to it.
			// NOTE: Need more checks here later.
			if( PrimitiveSection.Type_ == rsPT_LINELIST &&
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
			addPrimitive( rsPT_LINELIST, pFirstVertex, 2, Layer, BcTrue );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// drawLines
void ScnDebugRenderComponent::drawLines( const BcVec3d* pPoints, BcU32 NoofLines, const RsColour& Colour, BcU32 Layer )
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
			pVertices->ABGR_ = ABGR;
			++pVertices;
		}
		
		// Add primitive.		
		addPrimitive( rsPT_LINESTRIP, pFirstVertex, NoofVertices, Layer, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// drawGrid
void ScnDebugRenderComponent::drawGrid( const BcVec3d& Position, const BcVec3d& Size, BcF32 StepSize, BcF32 SubDivideMultiple, BcU32 Layer )
{
	BcU32 NoofAxis = ( Size.x() > 0 ? 1 : 0  ) + ( Size.y() > 0 ? 1 : 0  ) + ( Size.z() > 0 ? 1 : 0 );
	BcAssertMsg( NoofAxis == 2, "Only supports 2 axis in the grid!" );
	
	// Determine which axis to draw along.
	BcVec3d XAxis;
	BcVec3d YAxis;
	BcF32 XSize = 0.0f;
	BcF32 YSize = 0.0f;

	if( Size.x() > 0.0f )
	{
		XAxis = BcVec3d( Size.x(), 0.0f, 0.0f );
		XSize = Size.x();
		if( Size.y() > 0.0f )
		{
			YAxis = BcVec3d( 0.0f, Size.y(), 0.0f );
			YSize = Size.y();
		}
		else
		{
			YAxis = BcVec3d( 0.0f, 0.0f, Size.z() );
			YSize = Size.z();
		}
	}
	else
	{
		XAxis = BcVec3d( 0.0f, Size.y(), 0.0f );
		YAxis = BcVec3d( 0.0f, 0.0f, Size.z() );
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
			BcVec3d A1( Position + ( XAxis * X ) + ( YAxis *  YSize ) );
			BcVec3d B1( Position + ( XAxis * X ) + ( YAxis * -YSize ) );
			BcVec3d A2( Position + ( XAxis * -X ) + ( YAxis *  YSize ) );
			BcVec3d B2( Position + ( XAxis * -X ) + ( YAxis * -YSize ) );
			drawLine( A1, B1, RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), Layer );
			drawLine( A2, B2, RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), Layer );
		}

		for( BcF32 Y = 0.0f; Y <= YSize; Y += StepSize )
		{
			BcVec3d A1( Position + ( XAxis *  YSize ) + ( YAxis * Y ) );
			BcVec3d B1( Position + ( XAxis * -YSize ) + ( YAxis * Y ) );
			BcVec3d A2( Position + ( XAxis *  YSize ) + ( YAxis * -Y ) );
			BcVec3d B2( Position + ( XAxis * -YSize ) + ( YAxis * -Y ) );
			drawLine( A1, B1, RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), Layer );
			drawLine( A2, B2, RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), Layer );
		}

		StepSize *= SubDivideMultiple;
	}
}

//////////////////////////////////////////////////////////////////////////
// drawEllipsoid
void ScnDebugRenderComponent::drawEllipsoid( const BcVec3d& Position, const BcVec3d& Size, const RsColour& Colour, BcU32 Layer )
{
	// Draw outer circles for all axis.
	BcU32 LOD = 16;
	BcF32 Angle = 0.0f;
	BcF32 AngleInc = ( BcPI * 2.0f ) / BcF32( LOD );

	// Draw axis lines.
	for( BcU32 i = 0; i < LOD; ++i )
	{
		BcVec2d PosA( BcCos( Angle ), -BcSin( Angle ) );
		BcVec2d PosB( BcCos( Angle + AngleInc ), -BcSin( Angle + AngleInc ) );

		BcVec3d XAxisA = BcVec3d( 0.0f,                 PosA.x() * Size.y(), PosA.y() * Size.z() );
		BcVec3d YAxisA = BcVec3d( PosA.x() * Size.x(), 0.0f,                 PosA.y() * Size.z() );
		BcVec3d ZAxisA = BcVec3d( PosA.x() * Size.x(), PosA.y() * Size.y(), 0.0f                 );
		BcVec3d XAxisB = BcVec3d( 0.0f,                 PosB.x() * Size.y(), PosB.y() * Size.z() );
		BcVec3d YAxisB = BcVec3d( PosB.x() * Size.x(), 0.0f,                 PosB.y() * Size.z() );
		BcVec3d ZAxisB = BcVec3d( PosB.x() * Size.x(), PosB.y() * Size.y(), 0.0f                 );

		drawLine( XAxisA + Position, XAxisB + Position, Colour, 0 );
		drawLine( YAxisA + Position, YAxisB + Position, Colour, 0 );
		drawLine( ZAxisA + Position, ZAxisB + Position, Colour, 0 );

		Angle += AngleInc;
	}

	// Draw a cross down centre.
	BcVec3d XAxis = BcVec3d( Size.x(), 0.0f, 0.0f );
	BcVec3d YAxis = BcVec3d( 0.0f, Size.y(), 0.0f );
	BcVec3d ZAxis = BcVec3d( 0.0f, 0.0f, Size.z() );
	drawLine( Position - XAxis, Position + XAxis, Colour, Layer );
	drawLine( Position - YAxis, Position + YAxis, Colour, Layer );
	drawLine( Position - ZAxis, Position + ZAxis, Colour, Layer );
}

//////////////////////////////////////////////////////////////////////////
// drawAABB
void ScnDebugRenderComponent::drawAABB( const BcAABB& AABB, const RsColour& Colour, BcU32 Layer )
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
	// Set current render resource.
	pRenderResource_ = &RenderResources_[ CurrentRenderResource_ ];

	// Set vertices up.
	pVertices_ = pVerticesEnd_ = pRenderResource_->pVertices_;
	pVerticesEnd_ += NoofVertices_;
	VertexIndex_ = 0;
	
	// Empty primitive sections.
	PrimitiveSectionList_.clear();
	
	// Lock vertex buffer for use.
	if( HaveVertexBufferLock_ == BcFalse )
	{
		pRenderResource_->pVertexBuffer_->lock();
		HaveVertexBufferLock_ = BcTrue;
	}

	// Clear last primitive.
	LastPrimitiveSection_ = BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// preUpdate
//virtual
void ScnDebugRenderComponent::preUpdate( BcF32 Tick )
{
	Super::preUpdate( Tick );

	clear();

	//drawGrid( BcVec3d( 0.0f, 0.0f, 0.0f ), BcVec3d( 1000.0f, 0.0f, 1000.0f ), 1.0f, 10.0f, 0 );
}

//////////////////////////////////////////////////////////////////////////
// render
class ScnDebugRenderComponentRenderNode: public RsRenderNode
{
public:
	void render()
	{
		// TODO: Cache material instance so we don't rebind?
		for( BcU32 Idx = 0; Idx < NoofSections_; ++Idx )
		{
			ScnDebugRenderComponentPrimitiveSection* pPrimitiveSection = &pPrimitiveSections_[ Idx ];
			
			pPrimitive_->render( pPrimitiveSection->Type_, pPrimitiveSection->VertexIndex_, pPrimitiveSection->NoofVertices_ );
		}
	}
	
	BcU32 NoofSections_;
	ScnDebugRenderComponentPrimitiveSection* pPrimitiveSections_;
	RsPrimitive* pPrimitive_;
};

void ScnDebugRenderComponent::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{
	if( HaveVertexBufferLock_ == BcFalse )
	{
		return;
	}
	BcAssertMsg( HaveVertexBufferLock_ == BcTrue, "ScnDebugRenderComponent: Can't render without a vertex buffer lock." );

	// HUD pass.
	Sort.Layer_ = RS_SORT_LAYER_MAX;
	Sort.Pass_ = 0;

	// NOTE: Could do this sort inside of the renderer, but I'm just gonna keep the canvas
	//       as one solid object as to not conflict with other canvas objects when rendered
	//       to the scene. Will not sort by transparency or anything either.
	std::stable_sort( PrimitiveSectionList_.begin(), PrimitiveSectionList_.end(), ScnDebugRenderComponentPrimitiveSectionCompare() );
	
	// Cache last material instance.
	ScnMaterialComponent* pLastMaterialComponent = NULL;
	
	for( BcU32 Idx = 0; Idx < PrimitiveSectionList_.size(); ++Idx )
	{
		ScnDebugRenderComponentRenderNode* pRenderNode = pFrame->newObject< ScnDebugRenderComponentRenderNode >();
		
		pRenderNode->NoofSections_ = 1;//PrimitiveSectionList_.size();
		pRenderNode->pPrimitiveSections_ = pFrame->alloc< ScnDebugRenderComponentPrimitiveSection >( 1 );
		pRenderNode->pPrimitive_ = pRenderResource_->pPrimitive_;
		
		// Copy primitive sections in.
		BcMemCopy( pRenderNode->pPrimitiveSections_, &PrimitiveSectionList_[ Idx ], sizeof( ScnDebugRenderComponentPrimitiveSection ) * 1 );
		
		// Bind material.
		// NOTE: We should be binding for every single draw call. We can have the material deal with redundancy internally
		//       if need be. If using multiple canvases we could potentially lose a material bind.
		//if( pLastMaterialComponent != pRenderNode->pPrimitiveSections_->MaterialComponent_ )
		{
			pLastMaterialComponent = pRenderNode->pPrimitiveSections_->MaterialComponent_;
			// Set model parameters on material.
			pLastMaterialComponent->setWorldTransform( getParentEntity()->getWorldMatrix() );
			pViewComponent->setMaterialParameters( pLastMaterialComponent );

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
void ScnDebugRenderComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	getParentEntity()->attach( MaterialComponent_ );

	BcAssert( pImpl_ == NULL );
	pImpl_ = this;
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnDebugRenderComponent::onDetach( ScnEntityWeakRef Parent )
{
	getParentEntity()->detach( MaterialComponent_ );

	BcAssert( pImpl_ == this );
	pImpl_ = NULL;

	Super::onDetach( Parent );
}
