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
	if( !CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), MaterialComponent_, Material, ScnShaderPermutationFlags::MESH_PARTICLE_3D ) )
	{
		BcAssertMsg( BcFalse, "Material invalid blah." );
	}

	IsLocalSpace_ = Object["localspace"].asBool();

	// Cache texture bounds.
	ScnTextureRef Texture = Material->getTexture( "aDiffuseTex" );
	for( BcU32 Idx = 0; Idx < Texture->noofRects(); ++Idx )
	{
		ScnRect Rect = Texture->getRect( Idx );
		UVBounds_.push_back( MaVec4d( Rect.X_, Rect.Y_, Rect.X_ + Rect.W_, Rect.Y_ + Rect.H_ ) );
	}

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

	// Create vertex declaration.
	VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration( 
		RsVertexDeclarationDesc( 4 )
			.addElement( RsVertexElement( 0, 0,				3,		RsVertexDataType::FLOAT32,		RsVertexUsage::POSITION,		0 ) )
			.addElement( RsVertexElement( 0, 12,			3,		RsVertexDataType::FLOAT32,		RsVertexUsage::NORMAL,		0 ) )
			.addElement( RsVertexElement( 0, 24,			2,		RsVertexDataType::FLOAT32,		RsVertexUsage::TEXCOORD,		0 ) )
			.addElement( RsVertexElement( 0, 32,			4,		RsVertexDataType::UBYTE_NORM,	RsVertexUsage::COLOUR,		0 ) ) );

	// Allocate vertex buffers.
	for( BcU32 Idx = 0; Idx < 2; ++Idx )
	{
		TVertexBuffer& VertexBuffer = VertexBuffers_[ Idx ];
		VertexBuffer.pVertexBuffer_ = RsCore::pImpl()->createBuffer( 
			RsBufferDesc( 
				RsBufferType::VERTEX, 
				RsResourceCreationFlags::STREAM, 
				NoofVertices * sizeof( ScnParticleVertex ) ) );

		VertexBuffer.UniformBuffer_ = RsCore::pImpl()->createBuffer( 
			RsBufferDesc( 
				RsBufferType::UNIFORM,
				RsResourceCreationFlags::STREAM,
				sizeof( VertexBuffer.ObjectUniforms_ ) ) );
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
		RsCore::pImpl()->destroyResource( VertexBuffer.UniformBuffer_ );
	}

	RsCore::pImpl()->destroyResource( VertexDeclaration_ );
	
	delete [] pParticleBuffer_;
}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
MaAABB ScnParticleSystemComponent::getAABB() const
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
	SysKernel::pImpl()->pushDelegateJob( SysKernel::DEFAULT_JOB_QUEUE_ID, Delegate, Tick );
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
		pContext_->setVertexBuffer( 0, VertexBuffer_, sizeof( ScnParticleVertex ) );
		pContext_->setVertexDeclaration( VertexDeclaration_ );
		pContext_->drawPrimitives( RsTopologyType::TRIANGLE_LIST, 0, NoofIndices_ );
	}
	
	RsBuffer* VertexBuffer_;
	RsVertexDeclaration* VertexDeclaration_;
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

	// Calculate lock size.
	BcU32 NoofParticlesToRender = 0;
	for( BcU32 Idx = 0; Idx < NoofParticles_; ++Idx )
	{
		ScnParticle& Particle = pParticleBuffer_[ Idx ];

		if( Particle.Alive_ )
		{
			++NoofParticlesToRender;
		}
	}

	// Lock vertex buffer.
	UploadFence_.increment();
	RsCore::pImpl()->updateBuffer( 
		VertexBuffer.pVertexBuffer_,
		0,
		NoofParticlesToRender * sizeof( ScnParticleVertex ),
		RsResourceUpdateFlags::ASYNC,
		[ & ]
		( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			ScnParticleVertex* pVertex = reinterpret_cast< ScnParticleVertex* >( Lock.Buffer_ );
			for( BcU32 Idx = 0; Idx < NoofParticles_; ++Idx )
			{
				ScnParticle& Particle = pParticleBuffer_[ Idx ];

				if( Particle.Alive_ )
				{
					// Half size.
					const MaVec2d HalfSize = Particle.Scale_ * 0.5f;
					const BcF32 MaxHalfSize = BcMax( HalfSize.x(), HalfSize.y() );
					BcAssert( Particle.TextureIndex_ < UVBounds_.size() );
					const MaVec4d& UVBounds( UVBounds_[ Particle.TextureIndex_ ] );

					// Crappy rotation implementation :P
					const BcF32 Radians = Particle.Rotation_;
					MaVec2d CornerA = MaVec2d( -1.0f, -1.0f ) * HalfSize;
					MaVec2d CornerB = MaVec2d(  1.0f, -1.0f ) * HalfSize;
					MaVec2d CornerC = MaVec2d(  1.0f,  1.0f ) * HalfSize;
					MaVec2d CornerD = MaVec2d( -1.0f,  1.0f ) * HalfSize;
					if( Radians != NULL )
					{
						MaMat4d Rotation;
						Rotation.rotation( MaVec3d( 0.0f, 0.0f, Radians ) );
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
				}
			}

			UploadFence_.decrement();
		} );

	// Update uniform buffer.
	if( IsLocalSpace_ )
	{
		VertexBuffer.ObjectUniforms_.WorldTransform_ = getParentEntity()->getWorldMatrix();
	}
	else
	{
		VertexBuffer.ObjectUniforms_.WorldTransform_ = MaMat4d();
	}

	// Upload uniforms.
	RsCore::pImpl()->updateBuffer( 
		VertexBuffer.UniformBuffer_,
		0, sizeof( VertexBuffer.ObjectUniforms_ ),
		RsResourceUpdateFlags::ASYNC,
		[ this, VertexBuffer ]( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			BcMemCopy( Lock.Buffer_, &VertexBuffer.ObjectUniforms_, sizeof( VertexBuffer.ObjectUniforms_ ) );
		} );

	// Draw particles last.
	if( NoofParticlesToRender > 0 )
	{
		Sort.Layer_ = 15;

		// Set material parameters for view.
		pViewComponent->setMaterialParameters( MaterialComponent_ );

		// Bind material component.
		MaterialComponent_->bind( pFrame, Sort );

		// Setup render node.
		ScnParticleSystemComponentRenderNode* pRenderNode = pFrame->newObject< ScnParticleSystemComponentRenderNode >();
		pRenderNode->VertexBuffer_ = VertexBuffer.pVertexBuffer_;
		pRenderNode->VertexDeclaration_ = VertexDeclaration_;
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
			pParticle->Scale_ = MaVec2d( 0.0f, 0.0 );
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
	// Wait for upload to have completed.
	UploadFence_.wait();

	// TODO: Iterate over every "affector" at a time, rather than by particle.
	// - See "updateParticle".

	MaAABB FullAABB( MaVec3d( 0.0f, 0.0f, 0.0f ), MaVec3d( 0.0f, 0.0f, 0.0f ) );

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
			FullAABB.expandBy( Particle.Position_ - MaVec3d( MaxHalfSize, MaxHalfSize, MaxHalfSize ) );
			FullAABB.expandBy( Particle.Position_ + MaVec3d( MaxHalfSize, MaxHalfSize, MaxHalfSize ) );
		}
	}

	// Transform AABB.
	if( IsLocalSpace_ )
	{
		const MaMat4d& WorldTransform = getParentEntity()->getWorldMatrix();
		AABB_ = FullAABB.transform( WorldTransform );
	}
	else
	{
		AABB_ = FullAABB;
	}

	UpdateFence_.decrement();
}
