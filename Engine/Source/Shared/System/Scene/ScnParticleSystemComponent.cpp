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
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"

#include "Base/BcRandom.h"

//////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( ScnParticleSystemComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnParticleSystemComponent::initialise( const Json::Value& Object )
{
	// Grab number of particles.
	NoofParticles_ = Object["noofparticles"].asUInt();
	BcName MaterialName = Object["material"].asCString();

	ScnMaterialRef Material;
	if( !( CsCore::pImpl()->requestResource( BcName::NONE, MaterialName, Material ) && CsCore::pImpl()->createResource( BcName::NONE, MaterialComponent_, Material, BcErrorCode ) ) )
	{
		BcAssertMsg( BcFalse, "Material invalid blah." );
	}

	WorldTransformParam_ = MaterialComponent_->findParameter( "uWorldTransform" );
	
	// TODO: Allow different types of geom for this.
	// TODO: Use index buffer.
	// Calc what we need.
	BcU32 NoofVertices = NoofParticles_ * 6;	// 2x3 tris.
	BcU32 VertexDescriptor = rsVDF_POSITION_XYZ | rsVDF_NORMAL_XYZ | rsVDF_TEXCOORD_UV0 | rsVDF_COLOUR_RGBA8;

	// Allocate vertex buffers.
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TVertexBuffer& VertexBuffer = VertexBuffers_[ Idx ];
		VertexBuffer.pVertexArray_ =  new ScnParticleVertex[ NoofVertices ];
		VertexBuffer.pVertexBuffer_ = RsCore::pImpl()->createVertexBuffer( VertexDescriptor, NoofVertices, VertexBuffer.pVertexArray_ ); 
		VertexBuffer.pPrimitive_ = RsCore::pImpl()->createPrimitive( VertexBuffer.pVertexBuffer_, NULL );
	}

	// Allocate particles.
	pParticleBuffer_ = new ScnParticle[ NoofParticles_ ];
	BcMemZero( pParticleBuffer_, sizeof( ScnParticle ) * NoofParticles_ );

	// 
	CurrentVertexBuffer_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnParticleSystemComponent::destroy()
{
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TVertexBuffer& VertexBuffer = VertexBuffers_[ Idx ];
		delete [] VertexBuffer.pVertexArray_;
		RsCore::pImpl()->destroyResource( VertexBuffer.pVertexBuffer_ );
		RsCore::pImpl()->destroyResource( VertexBuffer.pPrimitive_ );
	}

	delete [] pParticleBuffer_;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnParticleSystemComponent::update( BcReal Tick )
{
	/*
	// TEST CODE.
	ScnParticle* pParticle = NULL;
	if( allocParticle( pParticle ) )
	{
		pParticle->Position_ = BcVec3d( 0.0f, 0.0f, 0.0f );
		pParticle->Velocity_ = BcVec3d( BcRandom::Global.randReal() - 0.5f, BcRandom::Global.randReal() - 0.25f, BcRandom::Global.randReal() - 0.5f ) * 8.0f;
		pParticle->Acceleration_ = BcVec3d( 0.0f, 0.0f, 0.0f );
		pParticle->MinScale_ = BcVec2d( 0.5f, 0.5f );
		pParticle->MaxScale_ = BcVec2d( 0.5f, 0.5f );
		pParticle->MinColour_ = RsColour( 1.0f, 0.0f, 1.0f, 1.0f );
		pParticle->MaxColour_ = RsColour( 0.0f, 1.0f, 1.0f, 0.0f );
		pParticle->UVBounds_ = BcVec4d( 0.0f, 0.0f, 1.0f, 1.0f );
		pParticle->CurrentTime_ = 0.0f;
		pParticle->MaxTime_ = 2.0f;
		pParticle->Alive_ = BcTrue;
	}
	//*/
	

	// Allocate particles.
	updateParticles( Tick );
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
void ScnParticleSystemComponent::render( RsFrame* pFrame, RsRenderSort Sort )
{
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
			// Use half size of particle. TODO: Ain't optimal.
			const BcVec2d HalfSize = Particle.Scale_ * 0.5f;
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
			VertexA.NX_ = -HalfSize.x();
			VertexA.NY_ = -HalfSize.y();
			VertexA.NZ_ = 0.0f;
			VertexA.U_ = Particle.UVBounds_.x();
			VertexA.V_ = Particle.UVBounds_.y();
			VertexA.RGBA_ = Colour;

			// 
			VertexB.X_ = Particle.Position_.x();
			VertexB.Y_ = Particle.Position_.y();
			VertexB.Z_ = Particle.Position_.z();
			VertexB.NX_ =  HalfSize.x();
			VertexB.NY_ = -HalfSize.y();
			VertexB.NZ_ = 0.0f;
			VertexB.U_ = Particle.UVBounds_.z();
			VertexB.V_ = Particle.UVBounds_.y();
			VertexB.RGBA_ = Colour;

			// 
			VertexC.X_ = Particle.Position_.x();
			VertexC.Y_ = Particle.Position_.y();
			VertexC.Z_ = Particle.Position_.z();
			VertexC.NX_ =  HalfSize.x();
			VertexC.NY_ =  HalfSize.y();
			VertexC.NZ_ = 0.0f;
			VertexC.U_ = Particle.UVBounds_.z();
			VertexC.V_ = Particle.UVBounds_.w();
			VertexC.RGBA_ = Colour;

			// 
			VertexD.X_ = Particle.Position_.x();
			VertexD.Y_ = Particle.Position_.y();
			VertexD.Z_ = Particle.Position_.z();
			VertexD.NX_ =  HalfSize.x();
			VertexD.NY_ =  HalfSize.y();
			VertexD.NZ_ = 0.0f;
			VertexD.U_ = Particle.UVBounds_.z();
			VertexD.V_ = Particle.UVBounds_.w();
			VertexD.RGBA_ = Colour;

			// 
			VertexE.X_ = Particle.Position_.x();
			VertexE.Y_ = Particle.Position_.y();
			VertexE.Z_ = Particle.Position_.z();
			VertexE.NX_ = -HalfSize.x();
			VertexE.NY_ =  HalfSize.y();
			VertexE.NZ_ = 0.0f;
			VertexE.U_ = Particle.UVBounds_.x();
			VertexE.V_ = Particle.UVBounds_.w();
			VertexE.RGBA_ = Colour;

			// 
			VertexF.X_ = Particle.Position_.x();
			VertexF.Y_ = Particle.Position_.y();
			VertexF.Z_ = Particle.Position_.z();
			VertexF.NX_ = -HalfSize.x();
			VertexF.NY_ = -HalfSize.y();
			VertexF.NZ_ = 0.0f;
			VertexF.U_ = Particle.UVBounds_.x();
			VertexF.V_ = Particle.UVBounds_.y();
			VertexF.RGBA_ = Colour;

			//
			++NoofParticlesToRender;
		}
	}

	// Update and unlock vertex buffer.	VertexBuffer.pVertexBuffer_->setNoofUpdateVertices( NoofParticlesToRender * 6 );
	VertexBuffer.pVertexBuffer_->unlock();

	// Bind material.
	MaterialComponent_->setParameter( WorldTransformParam_, BcMat4d() );
	MaterialComponent_->bind( pFrame, Sort );

	// Setup render node.
	ScnParticleSystemComponentRenderNode* pRenderNode = pFrame->newObject< ScnParticleSystemComponentRenderNode >();
	pRenderNode->pPrimitive_ = VertexBuffer.pPrimitive_;
	pRenderNode->NoofIndices_ = NoofParticlesToRender * 6;

	// Add to frame.
	pRenderNode->Sort_ = Sort;
	pFrame->addRenderNode( pRenderNode );
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

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// allocParticle
BcBool ScnParticleSystemComponent::allocParticle( ScnParticle*& pParticle )
{
	// TODO: Perhaps a free list of indices? Reordering of dead particles?
	//       Either way I want the update to be cache friendly.
	for( BcU32 Idx = 0; Idx < NoofParticles_; ++Idx )
	{
		BcU32 RealIdx = ( Idx + PotentialFreeParticle_ ) % NoofParticles_; // Slow. If we use powers of 2, we can &.
		ScnParticle& PotentiallyFreeParticle = pParticleBuffer_[ RealIdx ];

		if( PotentiallyFreeParticle.Alive_ == BcFalse )
		{
			pParticle = &PotentiallyFreeParticle;
			++PotentialFreeParticle_;
			return BcTrue;
		}
	}
	
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// updateParticle
void ScnParticleSystemComponent::updateParticle( ScnParticle& Particle, BcReal Tick )
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

	// Calculate interpolators.
	BcReal LerpValue = Particle.CurrentTime_ / Particle.MaxTime_;
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
void ScnParticleSystemComponent::updateParticles( BcReal Tick )
{
	// TODO: Iterate over every "affector" at a time, rather than by particle.
	// - See "updateParticle".

	// Not optimal, but clear code is clear. (For now...)
	for( BcU32 Idx = 0; Idx < NoofParticles_; ++Idx )
	{
		ScnParticle& Particle = pParticleBuffer_[ Idx ];

		if( Particle.Alive_ )
		{
			updateParticle( Particle, Tick );
		}
	}
}
