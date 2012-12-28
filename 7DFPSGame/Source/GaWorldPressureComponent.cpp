/**************************************************************************
*
* File:		GaWorldPressureComponent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		World Pressurce
*		
*
*
* 
**************************************************************************/

#include "GaWorldPressureComponent.h"

#define PROFILE_PRESSURE_UPDATE ( 0 )

//////////////////////////////////////////////////////////////////////////
// Define
DEFINE_RESOURCE( GaWorldPressureComponent );

BCREFLECTION_EMPTY_REGISTER( GaWorldPressureComponent );
/*
BCREFLECTION_DERIVED_BEGIN( ScnComponent, GaWorldPressureComponent )
	BCREFLECTION_MEMBER( BcName,							Name_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								Index_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( CsPackage,							pPackage_,						bcRFF_POINTER | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								RefCount_,						bcRFF_DEFAULT | bcRFF_TRANSIENT ),
BCREFLECTION_DERIVED_END();
*/

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaWorldPressureComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	IsReady_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void GaWorldPressureComponent::create__onAttach()
{
	//
	Width_ = BcPotRoundUp( WorldInfo_->getWidth(), 8 );
	Height_ = BcPotRoundUp( WorldInfo_->getHeight(), 8 );
	Depth_ = BcPotRoundUp( WorldInfo_->getDepth(), 8 );
	AccumMultiplier_ = WorldInfo_->getAccumulatorMultiplier() * WorldInfo_->getScale();
	Damping_ = WorldInfo_->getDamping() * WorldInfo_->getScale();

	Scale_ = WorldInfo_->getScale();
	Offset_ = BcVec2d( Width_ * Scale_, Height_ * Scale_ ) * -0.5f;

	// Allocate buffers.
	BufferSize_ = Width_ * Height_ * Depth_;
	pBuffers_[ 0 ] = new GaWorldPressureSample[ BufferSize_ ];
	pBuffers_[ 1 ] = new GaWorldPressureSample[ BufferSize_ ];
	BcMemZero( pBuffers_[ 0 ], sizeof( GaWorldPressureSample ) * BufferSize_ );
	BcMemZero( pBuffers_[ 1 ], sizeof( GaWorldPressureSample ) * BufferSize_ );
	CurrBuffer_ = 0;

	// Calc what we need.
	BcU32 NoofVertices = ( Width_ + Height_ + Depth_ ) * 6;
	BcU32 VertexDescriptor = rsVDF_POSITION_XYZ | rsVDF_TEXCOORD_UVW0;

	// Setup vertex buffer.
	pVertexArray_ =  new GaWorldPressureVertex[ NoofVertices ];

	GaWorldPressureVertex* pVertex = pVertexArray_;
	for( BcU32 Idx = 0; Idx < Width_; ++Idx )
	{
		BcF32 Position = (BcF32)Idx - (BcF32)( Width_ >> 1 );
		BcF32 Texture =  (BcF32)Idx / (BcF32)Width_;

		pVertex->X_ = Position;
		pVertex->Y_ = -(BcF32)( Height_ >> 1 );
		pVertex->Z_ = -(BcF32)( Depth_ >> 1 );
		pVertex->U_ = Texture;
		pVertex->V_ = 0.0f;
		pVertex->W_ = 0.0f;
		++pVertex;

		pVertex->X_ = Position;
		pVertex->Y_ = +(BcF32)( Height_ >> 1 );
		pVertex->Z_ = -(BcF32)( Depth_ >> 1 );
		pVertex->U_ = Texture;
		pVertex->V_ = 1.0f;
		pVertex->W_ = 0.0f;
		++pVertex;

		pVertex->X_ = Position;
		pVertex->Y_ = +(BcF32)( Height_ >> 1 );
		pVertex->Z_ = +(BcF32)( Depth_ >> 1 );
		pVertex->U_ = Texture;
		pVertex->V_ = 1.0f;
		pVertex->W_ = 1.0f;
		++pVertex;

		pVertex->X_ = Position;
		pVertex->Y_ = +(BcF32)( Height_ >> 1 );
		pVertex->Z_ = +(BcF32)( Depth_ >> 1 );
		pVertex->U_ = Texture;
		pVertex->V_ = 1.0f;
		pVertex->W_ = 1.0f;
		++pVertex;

		pVertex->X_ = Position;
		pVertex->Y_ = -(BcF32)( Height_ >> 1 );
		pVertex->Z_ = +(BcF32)( Depth_ >> 1 );
		pVertex->U_ = Texture;
		pVertex->V_ = 0.0f;
		pVertex->W_ = 1.0f;
		++pVertex;

		pVertex->X_ = Position;
		pVertex->Y_ = -(BcF32)( Height_ >> 1 );
		pVertex->Z_ = -(BcF32)( Depth_ >> 1 );
		pVertex->U_ = Texture;
		pVertex->V_ = 0.0f;
		pVertex->W_ = 0.0f;
		++pVertex;
	}

	for( BcU32 Idx = 0; Idx < Width_; ++Idx )
	{
		BcF32 Position = (BcF32)Idx - (BcF32)( Height_ >> 1 );
		BcF32 Texture =  (BcF32)Idx / (BcF32)Height_;

		pVertex->X_ = -(BcF32)( Width_ >> 1 );
		pVertex->Y_ = Position;
		pVertex->Z_ = -(BcF32)( Depth_ >> 1 );
		pVertex->U_ = 0.0f;
		pVertex->V_ = Texture;
		pVertex->W_ = 0.0f;
		++pVertex;

		pVertex->X_ = +(BcF32)( Width_ >> 1 );
		pVertex->Y_ = Position;
		pVertex->Z_ = -(BcF32)( Depth_ >> 1 );
		pVertex->U_ = 1.0f;
		pVertex->V_ = Texture;
		pVertex->W_ = 0.0f;
		++pVertex;

		pVertex->X_ = +(BcF32)( Width_ >> 1 );
		pVertex->Y_ = Position;
		pVertex->Z_ = +(BcF32)( Depth_ >> 1 );
		pVertex->U_ = 1.0f;
		pVertex->V_ = Texture;	
		pVertex->W_ = 1.0f;
		++pVertex;

		pVertex->X_ = +(BcF32)( Width_ >> 1 );
		pVertex->Y_ = Position;
		pVertex->Z_ = +(BcF32)( Depth_ >> 1 );
		pVertex->U_ = 1.0f;
		pVertex->V_ = Texture;	
		pVertex->W_ = 1.0f;
		++pVertex;

		pVertex->X_ = -(BcF32)( Width_ >> 1 );
		pVertex->Y_ = Position;
		pVertex->Z_ = +(BcF32)( Depth_ >> 1 );
		pVertex->U_ = 0.0f;
		pVertex->V_ = Texture;	
		pVertex->W_ = 1.0f;
		++pVertex;

		pVertex->X_ = -(BcF32)( Width_ >> 1 );
		pVertex->Y_ = Position;
		pVertex->Z_ = -(BcF32)( Depth_ >> 1 );
		pVertex->U_ = 0.0f;
		pVertex->V_ = Texture;	
		pVertex->W_ = 0.0f;
		++pVertex;
	}

	for( BcU32 Idx = 0; Idx < Depth_; ++Idx )
	{
		BcF32 Position = (BcF32)Idx - (BcF32)( Depth_ >> 1 );
		BcF32 Texture =  (BcF32)Idx / (BcF32)Depth_;

		pVertex->X_ = -(BcF32)( Width_ >> 1 );
		pVertex->Y_ = -(BcF32)( Height_ >> 1 );
		pVertex->Z_ = Position;
		pVertex->U_ = 0.0f;
		pVertex->V_ = 0.0f;
		pVertex->W_ = Texture;
		++pVertex;

		pVertex->X_ = +(BcF32)( Width_ >> 1 );
		pVertex->Y_ = -(BcF32)( Height_ >> 1 );
		pVertex->Z_ = Position;
		pVertex->U_ = 1.0f;
		pVertex->V_ = 0.0f;
		pVertex->W_ = Texture;
		++pVertex;

		pVertex->X_ = +(BcF32)( Width_ >> 1 );
		pVertex->Y_ = +(BcF32)( Height_ >> 1 );
		pVertex->Z_ = Position;
		pVertex->U_ = 1.0f;
		pVertex->V_ = 1.0f;
		pVertex->W_ = Texture;
		++pVertex;

		pVertex->X_ = +(BcF32)( Width_ >> 1 );
		pVertex->Y_ = +(BcF32)( Height_ >> 1 );
		pVertex->Z_ = Position;
		pVertex->U_ = 1.0f;
		pVertex->V_ = 1.0f;
		pVertex->W_ = Texture;
		++pVertex;

		pVertex->X_ = -(BcF32)( Width_ >> 1 );
		pVertex->Y_ = +(BcF32)( Height_ >> 1 );
		pVertex->Z_ = Position;
		pVertex->U_ = 0.0f;
		pVertex->V_ = 1.0f;
		pVertex->W_ = Texture;
		++pVertex;

		pVertex->X_ = -(BcF32)( Width_ >> 1 );
		pVertex->Y_ = -(BcF32)( Height_ >> 1 );
		pVertex->Z_ = Position;
		pVertex->U_ = 0.0f;
		pVertex->V_ = 0.0f;
		pVertex->W_ = Texture;
		++pVertex;
	}

	// Scale them to the sim size.
	for( BcU32 Idx = 0; Idx < NoofVertices; ++Idx )
	{
		pVertexArray_[ Idx ].X_ *= Scale_;
		pVertexArray_[ Idx ].Y_ *= Scale_;
		pVertexArray_[ Idx ].Z_ *= Scale_;
	}

	pVertexBuffer_ = RsCore::pImpl()->createVertexBuffer( VertexDescriptor, NoofVertices, pVertexArray_ ); 
	pPrimitive_ = RsCore::pImpl()->createPrimitive( pVertexBuffer_, NULL );

	CurrMaterial_ = 0;
	
	IsReady_ = BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void GaWorldPressureComponent::destroy__onDetach()
{
	RsCore::pImpl()->destroyResource( pVertexBuffer_ );
	RsCore::pImpl()->destroyResource( pPrimitive_ );

	// Wait for renderer.
	SysFence Fence( RsCore::WORKER_MASK );
	
	// Delete working data.
	delete [] pVertexArray_;
	
	delete [] pBuffers_[ 0 ];
	delete [] pBuffers_[ 1 ];
	pBuffers_[ 0 ] = NULL;
	pBuffers_[ 1 ] = NULL;

	IsReady_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
BcAABB GaWorldPressureComponent::getAABB() const 
{
	return BcAABB();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaWorldPressureComponent::update( BcF32 Tick )
{
	// Wait for update to have finished.
	UpdateFence_.wait();
	
	// Collide with BSP.
	collideSimulation();

	// Update texture.
	updateTexture();

	// Update glow textures.
	// TODO: Update once.
	// TODO: Post 7DFPS try this again.
	//updateGlowTextures();
	
	// Editor mode rendering.
	if( BSP_->InEditorMode_ )
	{
		Canvas_->setMaterialComponent( DynamicMaterials_[ CurrMaterial_ ].PreviewMaterial_ );
		BcVec2d HalfBoxSize( BcVec2d( (BcF32)Width_, (BcF32)Height_ ) * Scale_ * 0.5f );
		Canvas_->drawBox( -HalfBoxSize, HalfBoxSize, RsColour( 1.0f, 1.0f, 1.0f, 1.0f ), 0 );
	}
}

//////////////////////////////////////////////////////////////////////////
// render
class GaWorldPressureComponentRenderNode: public RsRenderNode
{
public:
	void render()
	{
		pPrimitive_->render( rsPT_TRIANGLELIST, 0, NoofIndices_ );
	}
	
	RsPrimitive* pPrimitive_;
	BcU32 NoofIndices_;
};

//virtual
void GaWorldPressureComponent::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{	
	//return;
	// Kick off the job to update the simulation in parallel to rendering..
	UpdateFence_.increment();
	BcDelegate< void(*)() > UpdateSimulationDelegate( BcDelegate< void(*)() >::bind< GaWorldPressureComponent, &GaWorldPressureComponent::updateSimulation >( this ) );
	SysKernel::pImpl()->enqueueDelegateJob< void(*)() >( SysKernel::USER_WORKER_MASK, UpdateSimulationDelegate );

	// Bind material and flip it.
	TDynamicMaterial& DynamicMaterial( DynamicMaterials_[ CurrMaterial_ ] );
	CurrMaterial_ = 1 - CurrMaterial_;
	DynamicMaterial.WorldMaterial_->setWorldTransform( BcMat4d() );
	pViewComponent->setMaterialParameters( DynamicMaterial.WorldMaterial_ );
	DynamicMaterial.WorldMaterial_->bind( pFrame, Sort );

	// Setup render node.
	GaWorldPressureComponentRenderNode* pRenderNode = pFrame->newObject< GaWorldPressureComponentRenderNode >();
	pRenderNode->pPrimitive_ = pPrimitive_;
	pRenderNode->NoofIndices_ = ( Width_ + Height_ + Depth_ ) * 6;

	// Add to frame.
	pRenderNode->Sort_ = Sort;	
	pFrame->addRenderNode( pRenderNode );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaWorldPressureComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Subscribe to reset event.
	GaWorldResetEvent::Delegate OnResetEvent = GaWorldResetEvent::Delegate::bind< GaWorldPressureComponent, &GaWorldPressureComponent::onReset >( this );
	Parent->subscribe( gaEVT_CORE_RESET, OnResetEvent );

	//
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >( 0 );
	WorldInfo_ = Parent->getComponentAnyParentByType< GaWorldInfoComponent >( 0 );
	BSP_ = Parent->getComponentAnyParentByType< GaWorldBSPComponent >( 0 );

	// Create stuff...
	create__onAttach();

	// Grab material
	ScnMaterialRef WorldMaterial;
	ScnMaterialRef PreviewMaterial;
	if( CsCore::pImpl()->requestResource( "materials", "air", WorldMaterial ) &&
		CsCore::pImpl()->requestResource( "materials", "airpreview", PreviewMaterial ) )
	{
		BcU32 TextureParam;
		for( BcU32 Idx = 0; Idx < 2; ++Idx )
		{
			TDynamicMaterial& DynamicMaterial( DynamicMaterials_[ Idx ] );
			
			// Create textures.
			CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), DynamicMaterial.WorldTexture1D_, Depth_, 1, rsTF_RGBA8 );
			CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), DynamicMaterial.WorldTexture2D_, Width_, Height_, 1, rsTF_RGBA8 );
			CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), DynamicMaterial.WorldTexture3D_, Width_, Height_, Depth_, 1, rsTF_RGBA8 );
			
			// Create material component, and attach textures.
			if( CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), DynamicMaterial.WorldMaterial_, WorldMaterial, scnSPF_3D ) )
			{
				TextureParam = DynamicMaterial.WorldMaterial_->findParameter( "aFloorTex" );
				DynamicMaterial.WorldMaterial_->setTexture( TextureParam, DynamicMaterial.WorldTexture1D_ );
				TextureParam = DynamicMaterial.WorldMaterial_->findParameter( "aWallTex" );
				DynamicMaterial.WorldMaterial_->setTexture( TextureParam, DynamicMaterial.WorldTexture2D_ );
				TextureParam = DynamicMaterial.WorldMaterial_->findParameter( "aDiffuseTex" );
				DynamicMaterial.WorldMaterial_->setTexture( TextureParam, DynamicMaterial.WorldTexture3D_ );
			}

			// Create material component, and attach textures.
			if( CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), DynamicMaterial.PreviewMaterial_, PreviewMaterial, scnSPF_2D ) )
			{
				TextureParam = DynamicMaterial.PreviewMaterial_->findParameter( "aFloorTex" );
				DynamicMaterial.PreviewMaterial_->setTexture( TextureParam, DynamicMaterial.WorldTexture1D_ );
				TextureParam = DynamicMaterial.PreviewMaterial_->findParameter( "aWallTex" );
				DynamicMaterial.PreviewMaterial_->setTexture( TextureParam, DynamicMaterial.WorldTexture2D_ );
				TextureParam = DynamicMaterial.PreviewMaterial_->findParameter( "aDiffuseTex" );
				DynamicMaterial.PreviewMaterial_->setTexture( TextureParam, DynamicMaterial.WorldTexture3D_ );
			}
		}
	}

	// Attach materials.
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TDynamicMaterial& DynamicMaterial( DynamicMaterials_[ Idx ] );
		Parent->attach( DynamicMaterial.PreviewMaterial_ );
		Parent->attach( DynamicMaterial.WorldMaterial_ );
	}
	
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaWorldPressureComponent::onDetach( ScnEntityWeakRef Parent )
{
	UpdateFence_.wait();

	destroy__onDetach();

	//
	Canvas_ = NULL;
	BSP_ = NULL;

	Parent->unsubscribeAll( this );

	// Detach materials.
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TDynamicMaterial& DynamicMaterial( DynamicMaterials_[ Idx ] );
		Parent->detach( DynamicMaterial.PreviewMaterial_ );
		Parent->detach( DynamicMaterial.WorldMaterial_ );
	}

	BcMemZero( DynamicMaterials_, sizeof( DynamicMaterials_ ) );

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// addSample
void GaWorldPressureComponent::addSample( const BcVec3d& Position, BcF32 Value )
{
	BcVec3d Offset = BcVec3d( Width_ * Scale_, Height_ * Scale_, Depth_ * Scale_ ) * 0.5f;
	BcVec3d Index = ( Position + Offset ) / Scale_;
	
	BcU32 X = (BcU32)BcClamp( Index.x(), 2, Width_ - 2 );
	BcU32 Y = (BcU32)BcClamp( Index.y(), 2, Height_ - 2 );
	BcU32 Z = (BcU32)BcClamp( Index.z(), 2, Depth_ - 2 );

	sample( CurrBuffer_, X, Y, Z ).Value_ += Value;
}
//////////////////////////////////////////////////////////////////////////
// setSample
void GaWorldPressureComponent::setSample( const BcVec3d& Position, BcF32 Value )
{
	BcVec3d Offset = BcVec3d( Width_ * Scale_, Height_ * Scale_, Depth_ * Scale_ ) * 0.5f;
	BcVec3d Index = ( Position + Offset ) / Scale_;
	
	BcU32 X = (BcU32)BcClamp( Index.x(), 3, Width_ - 3 );
	BcU32 Y = (BcU32)BcClamp( Index.y(), 3, Height_ - 3 );
	BcU32 Z = (BcU32)BcClamp( Index.z(), 3, Depth_  - 3 );

	sample( CurrBuffer_, X, Y, Z ).Value_ = Value;
	sample( CurrBuffer_, X - 1, Y, Z ).Value_ = Value * 0.5f;
	sample( CurrBuffer_, X + 1, Y, Z ).Value_ = Value * 0.5f;
	sample( CurrBuffer_, X, Y - 1, Z ).Value_ = Value * 0.5f;
	sample( CurrBuffer_, X, Y + 1, Z ).Value_ = Value * 0.5f;
	sample( CurrBuffer_, X, Y, Z - 1 ).Value_ = Value * 0.5f;
	sample( CurrBuffer_, X, Y, Z + 1 ).Value_ = Value * 0.5f;
}

//////////////////////////////////////////////////////////////////////////
// updateSimulation
void GaWorldPressureComponent::updateSimulation()
{
	BcScopedLogTimer ScopedTimer("Update Simulation");

	for( BcU32 Iters = 0; Iters < 2; ++Iters )
	{
		const register BcU32 WidthLessOne = Width_ - 1;
		const register BcU32 HeightLessOne = Height_ - 1;
		const register BcU32 DepthLessOne = Depth_ - 1;
		const register BcU32 W = Width_;
		const register BcU32 WH = Width_ * Height_;

		{
			BcScopedLogTimer ScopedTimer(" - Add random noise");
			for( BcU32 Idx = 0; Idx < 16; ++Idx )
			{
				BcU32 RandX = BcRandom::Global.randRange( 1, Width_ - 2 );
				BcU32 RandY = BcRandom::Global.randRange( 1, Height_ - 2 );
				BcU32 RandZ = 1;
				sample( CurrBuffer_, RandX, RandY, RandZ ).Value_ += 0.05f;
			}
		}
	
		// Update simulation.
		{
			BcScopedLogTimer ScopedTimer(" - Process");

			const BcU32 NextBuffer = 1 - CurrBuffer_;
			GaWorldPressureSample* pCurrBuffer = pBuffers_[ CurrBuffer_ ];
			GaWorldPressureSample* pNextBuffer = pBuffers_[ NextBuffer ];
			for( BcU32 Z = 1; Z < DepthLessOne; ++Z )
			{
				const BcU32 ZIdx = Z * ( Width_ * Height_ );
				for( BcU32 Y = 1; Y < HeightLessOne; ++Y )
				{
					const BcU32 YZIdx = Y * ( Width_ ) + ZIdx;
					register BcU32 XYZIdx = YZIdx;
					for( BcU32 X = 1; X < WidthLessOne; ++X )
					{
						++XYZIdx;
						GaWorldPressureSample& Output( pNextBuffer[ XYZIdx ] );
						register BcF32 Sample = pCurrBuffer[ XYZIdx - 1 ].Value_ +
												 pCurrBuffer[ XYZIdx + 1 ].Value_ +
												 pCurrBuffer[ XYZIdx - W ].Value_ +
												 pCurrBuffer[ XYZIdx + W ].Value_ +
												 pCurrBuffer[ XYZIdx - WH ].Value_ +
												 pCurrBuffer[ XYZIdx + WH ].Value_;
						Sample *= AccumMultiplier_;
						Sample -= Output.Value_;
						Output.Value_ = Sample - ( Sample * Damping_ );
					}
				}
			}
			CurrBuffer_ = NextBuffer;
		}
	}
	UpdateFence_.decrement();
}

//////////////////////////////////////////////////////////////////////////
// collideSimulation
void GaWorldPressureComponent::collideSimulation()
{
	BcScopedLogTimer ScopedTimer("Collide Simulation");

	const register BcU32 WidthLessOne = Width_ - 1;
	const register BcU32 HeightLessOne = Height_ - 1;
	const register BcU32 DepthLessOne = Depth_ - 1;
	const register BcU32 W = Width_;
	const register BcU32 WH = Width_ * Height_;

	GaWorldPressureSample* pCurrBuffer = pBuffers_[ CurrBuffer_ ];

	// Set solid areas from BSP.
	for( BcU32 Y = 1; Y < HeightLessOne; ++Y )
	{
		for( BcU32 X = 1; X < WidthLessOne; ++X )
		{
			BcVec3d Position( BcVec2d( (BcF32)X, (BcF32)Y ) * Scale_ + Offset_, 4.0f );
			if( BSP_->checkPointBack( Position, 0.0f ) )
			{
				const BcU32 XYIdx = X + Y * W;
				for( BcU32 Z = 1; Z < DepthLessOne; ++Z )
				{
					const BcU32 XYZIdx = ( Z * WH ) + XYIdx;
					GaWorldPressureSample& Output( pCurrBuffer[ XYZIdx ] );
					Output.Value_ *= 0.0f;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// updateTexture
void GaWorldPressureComponent::updateTexture()
{
	BcScopedLogTimer ScopedTimer("Update Texture");

	const BcF32 Brightness = 2.5f;
	// Update texture.
	TDynamicMaterial& DynamicMaterial( DynamicMaterials_[ CurrMaterial_ ] );
	RsTexture* pTexture = DynamicMaterial.WorldTexture3D_->getTexture();
	BcU32* pTexelData = reinterpret_cast< BcU32* >( pTexture->lockTexture() );
	GaWorldPressureSample* pInputBuffer = pBuffers_[ CurrBuffer_ ];

	BcU32 NoofTexels = Width_ * Height_ * Depth_;
	for( BcU32 Idx = 0; Idx < NoofTexels; ++Idx )
	{
		BcU32 Value = static_cast< BcU32 >( BcClamp( ( (*pInputBuffer++).Value_ ) * Brightness, 0.0f, 1.0f ) * 255.0f );
		BcU32 Colour = Value << 24 | 0x00ffffff;
		*pTexelData++ = Colour; 
	}
	pTexture->unlockTexture();
}

//////////////////////////////////////////////////////////////////////////
// updateGlowTextures
void GaWorldPressureComponent::updateGlowTextures()
{
	//for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TDynamicMaterial& DynamicMaterial( DynamicMaterials_[ CurrMaterial_ ] );
		
		// Update 1D texture.
		{
			RsTexture* pTexture = DynamicMaterial.WorldTexture1D_->getTexture();
			BcU32* pTexelData = reinterpret_cast< BcU32* >( pTexture->lockTexture() );
			BcU32 NoofTexels = Depth_;
			for( BcU32 Idx = 0; Idx < NoofTexels; ++Idx )
			{
				BcU32 Value = 255;
				BcU32 Colour = Value << 24 | 0x00ffffff;
				*pTexelData++ = Colour; 
			}
			pTexture->unlockTexture();
		}
		
		// Update 2D texture.
		{
			RsTexture* pTexture = DynamicMaterial.WorldTexture2D_->getTexture();
			BcU32* pTexelData = reinterpret_cast< BcU32* >( pTexture->lockTexture() );
		
			// Set solid areas from BSP.
			for( BcU32 Y = 0; Y < Height_; ++Y )
			{
				for( BcU32 X = 0; X < Width_; ++X )
				{
					BcVec3d Position( BcVec2d( (BcF32)X, (BcF32)Y ) * Scale_ + Offset_, 4.0f );
					BcU32 Value = 255;
					
					//if( BSP_->checkPointFront( Position, 0.25f ) )
					{
						Value = 255;
					}
					
					BcU32 Colour = Value << 24 | 0x00ffffff;
					*pTexelData++ = Value;
				}
			}

			pTexture->unlockTexture();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// onReset
eEvtReturn GaWorldPressureComponent::onReset( EvtID ID, const GaWorldResetEvent& Event )
{
	BcMemZero( pBuffers_[ 0 ], sizeof( GaWorldPressureSample ) * BufferSize_ );
	BcMemZero( pBuffers_[ 1 ], sizeof( GaWorldPressureSample ) * BufferSize_ );

	setSample( BcVec3d( Event.Position_, 0.0f ), 4.0f );

	return evtRET_PASS;
}
