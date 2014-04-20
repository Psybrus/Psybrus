/**************************************************************************
*
* File:		ScnParticleSystemComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	19/04/12
* Description:
*		Particle system component.
*		Renders and processes particle system.
*
*
* 
**************************************************************************/

#include "System/Scene/ScnParticleSystemComponent.h"
#include "System/Scene/ScnViewComponent.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"

#include "System/SysKernel.h"

#include "Base/BcRandom.h"
#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( ScnParticleSystemComponent );

void ScnParticleSystemComponent::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "MaterialComponent_",		&ScnParticleSystemComponent::MaterialComponent_ ),
	};
		
	ReRegisterClass< ScnParticleSystemComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -2060 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnParticleSystemComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	// Grab number of particles.
	NoofParticles_ = Object["noofparticles"].asUInt();
	ScnMaterialRef Material = getPackage()->getPackageCrossRef( Object["material"].asUInt() );
	if( !CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), MaterialComponent_, Material, scnSPF_PARTICLE_3D ) )
	{
		BcAssertMsg( BcFalse, "Material invalid blah." );
	}

	IsLocalSpace_ = Object["localspace"].asBool();

	// Cache texture bounds.
	ScnTextureRef Texture = Material->getTexture( "aDiffuseTex" );
	for( BcU32 Idx = 0; Idx < Texture->noofRects(); ++Idx )
	{
		ScnRect Rect = Texture->getRect( Idx );
		UVBounds_.push_back( BcVec4d( Rect.X_, Rect.Y_, Rect.X_ + Rect.W_, Rect.Y_ + Rect.H_ ) );
	}

	WorldTransformParam_ = MaterialComponent_->findParameter( "uWorldTransform" );

	BcMemZero( &VertexBuffers_, sizeof( VertexBuffers_ ) );
	pParticleBuffer_ = NULL;
	CurrentVertexBuffer_ = 0;

	PotentialFreeParticle_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnParticleSystemComponent::create()
{
	// TODO: Allow different types of geom for this.
	// TODO: Use index buffer.
	// Calc what we need.
	BcU32 NoofVertices = NoofParticles_ * 6;	// 2x3 tris.
	BcU32 VertexDescriptor = rsVDF_POSITION_XYZ | rsVDF_NORMAL_XYZ | rsVDF_TEXCOORD_UV0 | rsVDF_COLOUR_ABGR8;

	// Allocate vertex buffers.
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TVertexBuffer& VertexBuffer = VertexBuffers_[ Idx ];
		VertexBuffer.pVertexArray_ =  new ScnParticleVertex[ NoofVertices ];
		VertexBuffer.pVertexBuffer_ = RsCore::pImpl()->createVertexBuffer( RsVertexBufferDesc( VertexDescriptor, NoofVertices ), VertexBuffer.pVertexArray_ ); 
		VertexBuffer.pPrimitive_ = RsCore::pImpl()->createPrimitive( VertexBuffer.pVertexBuffer_, NULL );
	}

	// Allocate particles.
	pParticleBuffer_ = new ScnParticle[ NoofParticles_ ];
	BcMemZero( pParticleBuffer_, sizeof( ScnParticle ) * NoofParticles_ );
	
	Super::create();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnParticleSystemComponent::destroy()
{
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TVertexBuffer& VertexBuffer = VertexBuffers_[ Idx ];
		RsCore::pImpl()->destroyResource( VertexBuffer.pVertexBuffer_ );
		RsCore::pImpl()->destroyResource( VertexBuffer.pPrimitive_ );
	}

	// Wait for renderer.
	SysFence Fence( RsCore::WORKER_MASK );
	
	// Delete working data.
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TVertexBuffer& VertexBuffer = VertexBuffers_[ Idx ];
		delete [] VertexBuffer.pVertexArray_;
	}


	delete [] pParticleBuffer_;
}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
BcAABB ScnParticleSystemComponent::getAABB() const
{
	UpdateFence_.wait();

	return AABB_;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnParticleSystemComponent::postUpdate( BcF32 Tick )
{
	Super::postUpdate( Tick );

	UpdateFence_.increment();

#if 1
	typedef BcDelegate< void(*)( BcF32 ) > UpdateNodeDelegate;
	UpdateNodeDelegate Delegate = UpdateNodeDelegate::bind< ScnParticleSystemComponent, &ScnParticleSystemComponent::updateParticles >( this );
	SysKernel::pImpl()->enqueueDelegateJob( SysKernel::USER_WORKER_MASK, Delegate, Tick );
#else
	updateParticles( Tick );
#endif
}

//////////////////////////////////////////////////////////////////////////
// render
class ScnParticleSystemComponentRenderNode: public RsRenderNode
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
void ScnParticleSystemComponent::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{
	// Wait for update fence.
	UpdateFence_.wait();

	// Grab vertex buffer and flip for next frame to use.
	TVertexBuffer& VertexBuffer = VertexBuffers_[ CurrentVertexBuffer_ ];
	CurrentVertexBuffer_ = 1 - CurrentVertexBuffer_;

	// Lock vertex buffer.
	VertexBuffer.pVertexBuffer_->lock();

	// Iterate over alive particles, and setup vertex buffer.
	BcU32 NoofParticlesToRender = 0;
	ScnParticleVertex* pVertex = VertexBuffer.pVertexArray_;
	for( BcU32 Idx = 0; Idx < NoofParticles_; ++Idx )
	{
		ScnParticle& Particle = pParticleBuffer_[ Idx ];

		if( Particle.Alive_ )
		{
			// Half size.
			const BcVec2d HalfSize = Particle.Scale_ * 0.5f;
			const BcF32 MaxHalfSize = BcMax( HalfSize.x(), HalfSize.y() );
			BcAssert( Particle.TextureIndex_ < UVBounds_.size() );
			const BcVec4d& UVBounds( UVBounds_[ Particle.TextureIndex_ ] );

			// Crappy rotation implementation :P
			const BcF32 Radians = Particle.Rotation_;
			BcVec2d CornerA = BcVec2d( -1.0f, -1.0f ) * HalfSize;
			BcVec2d CornerB = BcVec2d(  1.0f, -1.0f ) * HalfSize;
			BcVec2d CornerC = BcVec2d(  1.0f,  1.0f ) * HalfSize;
			BcVec2d CornerD = BcVec2d( -1.0f,  1.0f ) * HalfSize;
			if( Radians != NULL )
			{
				BcMat4d Rotation;
				Rotation.rotation( BcVec3d( 0.0f, 0.0f, Radians ) );
				CornerA = CornerA * Rotation;
				CornerB = CornerB * Rotation;
				CornerC = CornerC * Rotation;
				CornerD = CornerD * Rotation;
			}

			const BcU32 Colour = Particle.Colour_.asABGR();

			// Grab vertices.
			ScnParticleVertex& VertexA = *pVertex++;
			ScnParticleVertex& VertexB = *pVertex++;
			ScnParticleVertex& VertexC = *pVertex++;

			ScnParticleVertex& VertexD = *pVertex++;
			ScnParticleVertex& VertexE = *pVertex++;
			ScnParticleVertex& VertexF = *pVertex++;
			
			// 
			VertexA.X_ = Particle.Position_.x();
			VertexA.Y_ = Particle.Position_.y();
			VertexA.Z_ = Particle.Position_.z();
			VertexA.NX_ = CornerA.x();
			VertexA.NY_ = CornerA.y();
			VertexA.NZ_ = 0.0f;
			VertexA.U_ = UVBounds.x();
			VertexA.V_ = UVBounds.y();
			VertexA.RGBA_ = Colour;

			// 
			VertexB.X_ = Particle.Position_.x();
			VertexB.Y_ = Particle.Position_.y();
			VertexB.Z_ = Particle.Position_.z();
			VertexB.NX_ = CornerB.x();
			VertexB.NY_ = CornerB.y();
			VertexB.NZ_ = 0.0f;
			VertexB.U_ = UVBounds.z();
			VertexB.V_ = UVBounds.y();
			VertexB.RGBA_ = Colour;

			// 
			VertexC.X_ = Particle.Position_.x();
			VertexC.Y_ = Particle.Position_.y();
			VertexC.Z_ = Particle.Position_.z();
			VertexC.NX_ = CornerC.x();
			VertexC.NY_ = CornerC.y();
			VertexC.NZ_ = 0.0f;
			VertexC.U_ = UVBounds.z();
			VertexC.V_ = UVBounds.w();
			VertexC.RGBA_ = Colour;

			// 
			VertexD.X_ = Particle.Position_.x();
			VertexD.Y_ = Particle.Position_.y();
			VertexD.Z_ = Particle.Position_.z();
			VertexD.NX_ = CornerC.x();
			VertexD.NY_ = CornerC.y();
			VertexD.NZ_ = 0.0f;
			VertexD.U_ = UVBounds.z();
			VertexD.V_ = UVBounds.w();
			VertexD.RGBA_ = Colour;

			// 
			VertexE.X_ = Particle.Position_.x();
			VertexE.Y_ = Particle.Position_.y();
			VertexE.Z_ = Particle.Position_.z();
			VertexE.NX_ = CornerD.x();
			VertexE.NY_ = CornerD.y();
			VertexE.NZ_ = 0.0f;
			VertexE.U_ = UVBounds.x();
			VertexE.V_ = UVBounds.w();
			VertexE.RGBA_ = Colour;

			// 
			VertexF.X_ = Particle.Position_.x();
			VertexF.Y_ = Particle.Position_.y();
			VertexF.Z_ = Particle.Position_.z();
			VertexF.NX_ = CornerA.x();
			VertexF.NY_ = CornerA.y();
			VertexF.NZ_ = 0.0f;
			VertexF.U_ = UVBounds.x();
			VertexF.V_ = UVBounds.y();
			VertexF.RGBA_ = Colour;
			
			//
			++NoofParticlesToRender;
		}
	}

	// Update and unlock vertex buffer.	
	VertexBuffer.pVertexBuffer_->setNoofUpdateVertices( NoofParticlesToRender * 6 );
	VertexBuffer.pVertexBuffer_->unlock();

	// Draw particles last.
	if( NoofParticlesToRender > 0 )
	{
		Sort.Layer_ = 15;

		// Bind material.
		if( IsLocalSpace_ )
		{
			const BcMat4d& WorldTransform = getParentEntity()->getWorldMatrix();
			MaterialComponent_->setParameter( WorldTransformParam_, WorldTransform );
		}
		else
		{
			MaterialComponent_->setParameter( WorldTransformParam_, BcMat4d() );
		}

		// Set material parameters for view.
		pViewComponent->setMaterialParameters( MaterialComponent_ );

		// Bind material component.
		MaterialComponent_->bind( pFrame, Sort );

		// Setup render node.
		ScnParticleSystemComponentRenderNode* pRenderNode = pFrame->newObject< ScnParticleSystemComponentRenderNode >();
		pRenderNode->pPrimitive_ = VertexBuffer.pPrimitive_;
		pRenderNode->NoofIndices_ = NoofParticlesToRender * 6;

		// Add to frame.
		pRenderNode->Sort_ = Sort;
		pFrame->addRenderNode( pRenderNode );
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnParticleSystemComponent::onAttach( ScnEntityWeakRef Parent )
{
	Parent->attach( MaterialComponent_ );

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnParticleSystemComponent::onDetach( ScnEntityWeakRef Parent )
{
	Parent->detach( MaterialComponent_ );

	MaterialComponent_ = NULL;

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// getMaterialComponent
ScnMaterialComponentRef ScnParticleSystemComponent::getMaterialComponent()
{
	return MaterialComponent_;
}

//////////////////////////////////////////////////////////////////////////
// allocParticle
BcBool ScnParticleSystemComponent::allocParticle( ScnParticle*& pParticle )
{
	// We can't be allocating whilst we're updating.
	BcAssert( UpdateFence_.count() == 0 );

	// TODO: Perhaps a free list of indices? Reordering of dead particles?
	//       Either way I want the update to be cache friendly.
	for( BcU32 Idx = 0; Idx < NoofParticles_; ++Idx )
	{
		BcU32 RealIdx = ( Idx + PotentialFreeParticle_ ) % NoofParticles_; // Slow. If we use powers of 2, we can &.
		ScnParticle* pPotentiallyFreeParticle = &pParticleBuffer_[ RealIdx ];

		if( pPotentiallyFreeParticle->Alive_ == BcFalse )
		{
			pParticle = pPotentiallyFreeParticle;

			// Prevent it being rendered for the first frame.
			pParticle->Scale_ = BcVec2d( 0.0f, 0.0 );
			pParticle->Colour_ = RsColour( 0.0f, 0.0f, 0.0f, 0.0f );

			++PotentialFreeParticle_;
			return BcTrue;
		}
	}
	
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// updateParticle
void ScnParticleSystemComponent::updateParticle( ScnParticle& Particle, BcF32 Tick )
{
	// TODO: Move each section of this into "affectors":
	// - Physics affector: Move physically based on vel/accel.
	// - Colour lerp affector: Some more advanced colour interpolation.
	// - Maybe even fancier stuff like curve based interpolation of position, colour, scale?
	// - - Advantage: Min/Max and that stuff can live in the affector (memory saving++).
	// For now, KISS. This just needs to function, I can make it better later :)
	
	BcAssert( Particle.Alive_ == BcTrue );
	
	// Do position.
	Particle.Position_ += Particle.Velocity_ * Tick;
	Particle.Velocity_ += Particle.Acceleration_ * Tick;

	// Do rotation.
	Particle.Rotation_ += Particle.RotationMultiplier_ * Tick;

	// Calculate interpolators.
	BcF32 LerpValue = Particle.CurrentTime_ / Particle.MaxTime_;
	Particle.Scale_.lerp( Particle.MinScale_, Particle.MaxScale_, LerpValue );
	Particle.Colour_.lerp( Particle.MinColour_, Particle.MaxColour_, LerpValue );
	
	// Advance current time.
	Particle.CurrentTime_ += Tick;
	
	// Kill particle!
	if( Particle.CurrentTime_ > Particle.MaxTime_ )
	{
		Particle.Alive_ = BcFalse;
	}
}

//////////////////////////////////////////////////////////////////////////
// updateParticles
void ScnParticleSystemComponent::updateParticles( BcF32 Tick )
{
	// TODO: Iterate over every "affector" at a time, rather than by particle.
	// - See "updateParticle".

	BcAABB FullAABB( BcVec3d( 0.0f, 0.0f, 0.0f ), BcVec3d( 0.0f, 0.0f, 0.0f ) );

	// Not optimal, but clear code is clear. (For now...)
	for( BcU32 Idx = 0; Idx < NoofParticles_; ++Idx )
	{
		ScnParticle& Particle = pParticleBuffer_[ Idx ];

		if( Particle.Alive_ )
		{
			// Update particle.
			updateParticle( Particle, Tick );

			// Expand AABB by particle's max bounds.
			const BcF32 MaxHalfSize = BcMax( Particle.Scale_.x(), Particle.Scale_.y() ) * 0.5f;
			FullAABB.expandBy( Particle.Position_ - BcVec3d( MaxHalfSize, MaxHalfSize, MaxHalfSize ) );
			FullAABB.expandBy( Particle.Position_ + BcVec3d( MaxHalfSize, MaxHalfSize, MaxHalfSize ) );
		}
	}

	// Transform AABB.
	if( IsLocalSpace_ )
	{
		const BcMat4d& WorldTransform = getParentEntity()->getWorldMatrix();
		AABB_ = FullAABB.transform( WorldTransform );
	}
	else
	{
		AABB_ = FullAABB;
	}

	UpdateFence_.decrement();
}
