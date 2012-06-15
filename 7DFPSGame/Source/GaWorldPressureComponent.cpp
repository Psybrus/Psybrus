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

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaWorldPressureComponent::initialise( const Json::Value& Object )
{
	Width_ = 128;
	Height_ = 128;
	Depth_ = 8;
	AccumMultiplier_ = 0.31f;
	Damping_ = 0.02f;

	Scale_ = 0.25f;
	Offset_ = BcVec2d( Width_ * Scale_, Height_ * Scale_ ) * -0.5f;

	BufferSize_ = Width_ * Height_ * Depth_;
	pBuffers_[ 0 ] = new GaWorldPressureSample[ BufferSize_ ];
	pBuffers_[ 1 ] = new GaWorldPressureSample[ BufferSize_ ];
	BcMemZero( pBuffers_[ 0 ], sizeof( GaWorldPressureSample ) * BufferSize_ );
	BcMemZero( pBuffers_[ 1 ], sizeof( GaWorldPressureSample ) * BufferSize_ );
	CurrBuffer_ = 0;

	IsReady_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void GaWorldPressureComponent::create()
{
	// Calc what we need.
	BcU32 NoofVertices = ( Width_ + Height_ + Depth_ ) * 6;
	BcU32 VertexDescriptor = rsVDF_POSITION_XYZ | rsVDF_TEXCOORD_UVW0;

	// Setup vertex buffer.
	pVertexArray_ =  new GaWorldPressureVertex[ NoofVertices ];

	GaWorldPressureVertex* pVertex = pVertexArray_;
	for( BcU32 Idx = 0; Idx < Width_; ++Idx )
	{
		BcReal Position = (BcReal)Idx - (BcReal)( Width_ >> 1 );
		BcReal Texture =  (BcReal)Idx / (BcReal)Width_;

		pVertex->X_ = Position;
		pVertex->Y_ = -(BcReal)( Height_ >> 1 );
		pVertex->Z_ = -(BcReal)( Depth_ >> 1 );
		pVertex->U_ = Texture;
		pVertex->V_ = 0.0f;
		pVertex->W_ = 0.0f;
		++pVertex;

		pVertex->X_ = Position;
		pVertex->Y_ = +(BcReal)( Height_ >> 1 );
		pVertex->Z_ = -(BcReal)( Depth_ >> 1 );
		pVertex->U_ = Texture;
		pVertex->V_ = 1.0f;
		pVertex->W_ = 0.0f;
		++pVertex;

		pVertex->X_ = Position;
		pVertex->Y_ = +(BcReal)( Height_ >> 1 );
		pVertex->Z_ = +(BcReal)( Depth_ >> 1 );
		pVertex->U_ = Texture;
		pVertex->V_ = 1.0f;
		pVertex->W_ = 1.0f;
		++pVertex;

		pVertex->X_ = Position;
		pVertex->Y_ = +(BcReal)( Height_ >> 1 );
		pVertex->Z_ = +(BcReal)( Depth_ >> 1 );
		pVertex->U_ = Texture;
		pVertex->V_ = 1.0f;
		pVertex->W_ = 1.0f;
		++pVertex;

		pVertex->X_ = Position;
		pVertex->Y_ = -(BcReal)( Height_ >> 1 );
		pVertex->Z_ = +(BcReal)( Depth_ >> 1 );
		pVertex->U_ = Texture;
		pVertex->V_ = 0.0f;
		pVertex->W_ = 1.0f;
		++pVertex;

		pVertex->X_ = Position;
		pVertex->Y_ = -(BcReal)( Height_ >> 1 );
		pVertex->Z_ = -(BcReal)( Depth_ >> 1 );
		pVertex->U_ = Texture;
		pVertex->V_ = 0.0f;
		pVertex->W_ = 0.0f;
		++pVertex;
	}

	for( BcU32 Idx = 0; Idx < Width_; ++Idx )
	{
		BcReal Position = (BcReal)Idx - (BcReal)( Height_ >> 1 );
		BcReal Texture =  (BcReal)Idx / (BcReal)Height_;

		pVertex->X_ = -(BcReal)( Width_ >> 1 );
		pVertex->Y_ = Position;
		pVertex->Z_ = -(BcReal)( Depth_ >> 1 );
		pVertex->U_ = 0.0f;
		pVertex->V_ = Texture;
		pVertex->W_ = 0.0f;
		++pVertex;

		pVertex->X_ = +(BcReal)( Width_ >> 1 );
		pVertex->Y_ = Position;
		pVertex->Z_ = -(BcReal)( Depth_ >> 1 );
		pVertex->U_ = 1.0f;
		pVertex->V_ = Texture;
		pVertex->W_ = 0.0f;
		++pVertex;

		pVertex->X_ = +(BcReal)( Width_ >> 1 );
		pVertex->Y_ = Position;
		pVertex->Z_ = +(BcReal)( Depth_ >> 1 );
		pVertex->U_ = 1.0f;
		pVertex->V_ = Texture;	
		pVertex->W_ = 1.0f;
		++pVertex;

		pVertex->X_ = +(BcReal)( Width_ >> 1 );
		pVertex->Y_ = Position;
		pVertex->Z_ = +(BcReal)( Depth_ >> 1 );
		pVertex->U_ = 1.0f;
		pVertex->V_ = Texture;	
		pVertex->W_ = 1.0f;
		++pVertex;

		pVertex->X_ = -(BcReal)( Width_ >> 1 );
		pVertex->Y_ = Position;
		pVertex->Z_ = +(BcReal)( Depth_ >> 1 );
		pVertex->U_ = 0.0f;
		pVertex->V_ = Texture;	
		pVertex->W_ = 1.0f;
		++pVertex;

		pVertex->X_ = -(BcReal)( Width_ >> 1 );
		pVertex->Y_ = Position;
		pVertex->Z_ = -(BcReal)( Depth_ >> 1 );
		pVertex->U_ = 0.0f;
		pVertex->V_ = Texture;	
		pVertex->W_ = 0.0f;
		++pVertex;
	}

	for( BcU32 Idx = 0; Idx < Depth_; ++Idx )
	{
		BcReal Position = (BcReal)Idx - (BcReal)( Depth_ >> 1 );
		BcReal Texture =  (BcReal)Idx / (BcReal)Depth_;

		pVertex->X_ = -(BcReal)( Width_ >> 1 );
		pVertex->Y_ = -(BcReal)( Height_ >> 1 );
		pVertex->Z_ = Position;
		pVertex->U_ = 0.0f;
		pVertex->V_ = 0.0f;
		pVertex->W_ = Texture;
		++pVertex;

		pVertex->X_ = +(BcReal)( Width_ >> 1 );
		pVertex->Y_ = -(BcReal)( Height_ >> 1 );
		pVertex->Z_ = Position;
		pVertex->U_ = 1.0f;
		pVertex->V_ = 0.0f;
		pVertex->W_ = Texture;
		++pVertex;

		pVertex->X_ = +(BcReal)( Width_ >> 1 );
		pVertex->Y_ = +(BcReal)( Height_ >> 1 );
		pVertex->Z_ = Position;
		pVertex->U_ = 1.0f;
		pVertex->V_ = 1.0f;
		pVertex->W_ = Texture;
		++pVertex;

		pVertex->X_ = +(BcReal)( Width_ >> 1 );
		pVertex->Y_ = +(BcReal)( Height_ >> 1 );
		pVertex->Z_ = Position;
		pVertex->U_ = 1.0f;
		pVertex->V_ = 1.0f;
		pVertex->W_ = Texture;
		++pVertex;

		pVertex->X_ = -(BcReal)( Width_ >> 1 );
		pVertex->Y_ = +(BcReal)( Height_ >> 1 );
		pVertex->Z_ = Position;
		pVertex->U_ = 0.0f;
		pVertex->V_ = 1.0f;
		pVertex->W_ = Texture;
		++pVertex;

		pVertex->X_ = -(BcReal)( Width_ >> 1 );
		pVertex->Y_ = -(BcReal)( Height_ >> 1 );
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
void GaWorldPressureComponent::destroy()
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
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool GaWorldPressureComponent::isReady()
{
	return IsReady_;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaWorldPressureComponent::update( BcReal Tick )
{
#if PROFILE_PRESSURE_UPDATE
	BcTimer Timer;
	Timer.mark();
#endif
	
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

	// Kick off the job to update the simulation asynchronously.
	UpdateFence_.increment();
	BcDelegate< void(*)() > UpdateSimulationDelegate( BcDelegate< void(*)() >::bind< GaWorldPressureComponent, &GaWorldPressureComponent::updateSimulation >( this ) );
	SysKernel::pImpl()->enqueueDelegateJob< void(*)() >( SysKernel::USER_WORKER_MASK, UpdateSimulationDelegate );

	// HACK HACK HACK.
	UpdateFence_.wait();
	
	// Editor mode rendering.
	if( BSP_->InEditorMode_ )
	{
		Canvas_->setMaterialComponent( DynamicMaterials_[ CurrMaterial_ ].PreviewMaterial_ );
		BcVec2d HalfBoxSize( BcVec2d( (BcReal)Width_, (BcReal)Height_ ) * Scale_ * 0.5f );
		Canvas_->drawBox( -HalfBoxSize, HalfBoxSize, RsColour( 1.0f, 1.0f, 1.0f, 1.0f ), 0 );
	}

#if PROFILE_PRESSURE_UPDATE
	BcReal Time = Timer.time();
	BcPrintf("GaWorldPressureComponent Time: %.2f ms\n", Time * 1000.0f);
#endif
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
	// Bind material and flip it.
	TDynamicMaterial& DynamicMaterial( DynamicMaterials_[ CurrMaterial_ ] );
	CurrMaterial_ = 1 - CurrMaterial_;
	DynamicMaterial.WorldMaterial_->setWorldTransform( BcMat4d() );
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
	//
	Canvas_ = Parent->getComponentByType< ScnCanvasComponent >( 0 );
	BSP_ = Parent->getComponentByType< GaWorldBSPComponent >( 0 );

	// Grab material
	ScnMaterialRef WorldMaterial;
	ScnMaterialRef PreviewMaterial;
	if( CsCore::pImpl()->requestResource( "default", "air", WorldMaterial ) &&
		CsCore::pImpl()->requestResource( "default", "airpreview", PreviewMaterial ) )
	{
		BcU32 TextureParam;
		for( BcU32 Idx = 0; Idx < 2; ++Idx )
		{
			TDynamicMaterial& DynamicMaterial( DynamicMaterials_[ Idx ] );
			
			// Create textures.
			CsCore::pImpl()->createResource( BcName::INVALID, DynamicMaterial.WorldTexture1D_, Depth_, 1, rsTF_RGBA8 );
			CsCore::pImpl()->createResource( BcName::INVALID, DynamicMaterial.WorldTexture2D_, Width_, Height_, 1, rsTF_RGBA8 );
			CsCore::pImpl()->createResource( BcName::INVALID, DynamicMaterial.WorldTexture3D_, Width_, Height_, Depth_, 1, rsTF_RGBA8 );
			
			// Create material component, and attach textures.
			if( CsCore::pImpl()->createResource( BcName::INVALID, DynamicMaterial.WorldMaterial_, WorldMaterial, BcErrorCode ) )
			{
				TextureParam = DynamicMaterial.WorldMaterial_->findParameter( "aFloorTex" );
				DynamicMaterial.WorldMaterial_->setTexture( TextureParam, DynamicMaterial.WorldTexture1D_ );
				TextureParam = DynamicMaterial.WorldMaterial_->findParameter( "aWallTex" );
				DynamicMaterial.WorldMaterial_->setTexture( TextureParam, DynamicMaterial.WorldTexture2D_ );
				TextureParam = DynamicMaterial.WorldMaterial_->findParameter( "aDiffuseTex" );
				DynamicMaterial.WorldMaterial_->setTexture( TextureParam, DynamicMaterial.WorldTexture3D_ );
			}

			// Create material component, and attach textures.
			if( CsCore::pImpl()->createResource( BcName::INVALID, DynamicMaterial.PreviewMaterial_, PreviewMaterial, BcErrorCode ) )
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
	Canvas_ = NULL;
	BSP_ = NULL;

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
void GaWorldPressureComponent::addSample( const BcVec3d& Position, BcReal Value )
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
void GaWorldPressureComponent::setSample( const BcVec3d& Position, BcReal Value )
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
	//for( BcU32 Iters = 0; Iters < 2; ++Iters )
	{
		const register BcU32 WidthLessOne = Width_ - 1;
		const register BcU32 HeightLessOne = Height_ - 1;
		const register BcU32 DepthLessOne = Depth_ - 1;
		const register BcU32 W = Width_;
		const register BcU32 WH = Width_ * Height_;

		for( BcU32 Idx = 0; Idx < 2; ++Idx )
		{
			BcU32 RandX = BcRandom::Global.randRange( 1, Width_ - 2 );
			BcU32 RandY = BcRandom::Global.randRange( 1, Height_ - 2 );
			BcU32 RandZ = 1;
			sample( CurrBuffer_, RandX, RandY, RandZ ).Value_ += 0.05f;
		}
	
		// Update simulation.
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
					register BcReal Sample = pCurrBuffer[ XYZIdx - 1 ].Value_ +
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
	UpdateFence_.decrement();
}

//////////////////////////////////////////////////////////////////////////
// collideSimulation
void GaWorldPressureComponent::collideSimulation()
{
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
			BcVec3d Position( BcVec2d( (BcReal)X, (BcReal)Y ) * Scale_ + Offset_, 4.0f );
			if( BSP_->checkPointBack( Position, 0.0f ) )
			{
				const BcU32 XYIdx = X + Y * W;
				for( BcU32 Z = 1; Z < DepthLessOne; ++Z )
				{
					const BcU32 XYZIdx = ( Z * WH ) + XYIdx;
					GaWorldPressureSample& Output( pCurrBuffer[ XYZIdx ] );
					Output.Value_ = 0.0f;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// updateTexture
void GaWorldPressureComponent::updateTexture()
{
	const BcReal Brightness = 2.5f;
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
					BcVec3d Position( BcVec2d( (BcReal)X, (BcReal)Y ) * Scale_ + Offset_, 4.0f );
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
