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

#include "System/Scene/Rendering/ScnParticleSystemComponent.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnViewRenderData.h"

#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"

#include "System/SysKernel.h"

#include "Base/BcRandom.h"
#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// ScnParticleSystemViewRenderData
class ScnParticleSystemViewRenderData : 
	public ScnViewRenderData
{
public:
	ScnParticleSystemViewRenderData()
	{
	}

	virtual ~ScnParticleSystemViewRenderData()
	{
	}

	RsProgramBindingUPtr ProgramBinding_;
	RsRenderState* RenderState_;
};

//////////////////////////////////////////////////////////////////////////
// Define resource.
REFLECTION_DEFINE_DERIVED( ScnParticleSystemComponent );

void ScnParticleSystemComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "NoofParticles_", &ScnParticleSystemComponent::NoofParticles_, bcRFF_IMPORTER | bcRFF_CONST ),
		new ReField( "IsLocalSpace_", &ScnParticleSystemComponent::IsLocalSpace_, bcRFF_IMPORTER ),
		new ReField( "Material_", &ScnParticleSystemComponent::Material_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),

		new ReField( "VertexDeclaration_", &ScnParticleSystemComponent::VertexDeclaration_, bcRFF_TRANSIENT ),
		new ReField( "ObjectUniforms_", &ScnParticleSystemComponent::ObjectUniforms_, bcRFF_TRANSIENT ),
		new ReField( "pParticleBuffer_", &ScnParticleSystemComponent::pParticleBuffer_, bcRFF_TRANSIENT ),
		new ReField( "PotentialFreeParticle_", &ScnParticleSystemComponent::PotentialFreeParticle_, bcRFF_TRANSIENT ),
		new ReField( "WorldTransformParam_", &ScnParticleSystemComponent::WorldTransformParam_, bcRFF_TRANSIENT ),
		new ReField( "UVBounds_", &ScnParticleSystemComponent::UVBounds_ ),
		new ReField( "AABB_", &ScnParticleSystemComponent::AABB_ ),
	};
	
	using namespace std::placeholders;
	ReRegisterClass< ScnParticleSystemComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( 
			{
				ScnComponentProcessFuncEntry(
					"Update Particles",
					ScnComponentPriority::PARTICLE_SYSTEM_UPDATE,
					std::bind( &ScnParticleSystemComponent::updateParticles, _1 ) ),
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnParticleSystemComponent::ScnParticleSystemComponent():
	VertexDeclaration_( nullptr ),
	pParticleBuffer_( nullptr ),
	NoofParticles_( 0 ),
	PotentialFreeParticle_( 0 ),
	Material_( nullptr ),
	WorldTransformParam_( 0 ),
	IsLocalSpace_( BcFalse )
{
	BcMemZero( &ObjectUniforms_, sizeof( ObjectUniforms_ ) );
	setPasses( RsRenderSortPassFlags::TRANSPARENT );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnParticleSystemComponent::~ScnParticleSystemComponent()
{
	//SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnParticleSystemComponent::initialise()
{
	Super::initialise();

	// Cache texture bounds.
	if( Material_ != nullptr )
	{
		ScnTextureRef Texture = Material_->getTexture( "aDiffuseTex" );
		if( Texture.isValid() )
		{
			UVBounds_.reserve( Texture->noofRects() );
			for( BcU32 Idx = 0; Idx < Texture->noofRects(); ++Idx )
			{
				ScnRect Rect = Texture->getRect( Idx );
				UVBounds_.push_back( MaVec4d( Rect.X_, Rect.Y_, Rect.X_ + Rect.W_, Rect.Y_ + Rect.H_ ) );
			}
		}
		else
		{
			UVBounds_.push_back( MaVec4d( 0.0f, 0.0f, 1.0f, 1.0f ) );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
MaAABB ScnParticleSystemComponent::getAABB() const
{
	return AABB_;
}

//////////////////////////////////////////////////////////////////////////
// render
//virtual
void ScnParticleSystemComponent::render( ScnRenderContext & RenderContext )
{
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
	if( NoofParticlesToRender > 0 )
	{
		UploadFence_.increment();
		RsCore::pImpl()->updateBuffer( 
			VertexBuffer_.get(),
			0,
			NoofParticlesToRender * sizeof( ScnParticleVertex ) * 6,
			RsResourceUpdateFlags::ASYNC,
			[ this, NoofParticlesToRender ]
			( RsBuffer* Buffer, const RsBufferLock& Lock )
			{
				BcU32 NoofParticlesRendered = 0;
				ScnParticleVertex* pVertex = reinterpret_cast< ScnParticleVertex* >( Lock.Buffer_ );
				for( BcU32 Idx = 0; Idx < NoofParticles_; ++Idx )
				{
					ScnParticle& Particle = pParticleBuffer_[ Idx ];

					if( Particle.Alive_ )
					{
						++NoofParticlesRendered;

						// Half size.
						const MaVec2d HalfSize = Particle.Scale_ * 0.5f;
						BcAssert( Particle.TextureIndex_ < UVBounds_.size() );
						const MaVec4d& UVBounds( UVBounds_[ Particle.TextureIndex_ ] );

						// Crappy rotation implementation :P
						const BcF32 Radians = Particle.Rotation_;
						MaVec2d CornerA = MaVec2d( -1.0f, -1.0f ) * HalfSize;
						MaVec2d CornerB = MaVec2d(  1.0f, -1.0f ) * HalfSize;
						MaVec2d CornerC = MaVec2d(  1.0f,  1.0f ) * HalfSize;
						MaVec2d CornerD = MaVec2d( -1.0f,  1.0f ) * HalfSize;
						
						if( Radians != 0.0f )
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
						VertexA.W_ = 1.0f;
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
						VertexB.W_ = 1.0f;
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
						VertexC.W_ = 1.0f;
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
						VertexD.W_ = 1.0f;
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
						VertexE.W_ = 1.0f;
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
						VertexF.W_ = 1.0f;
						VertexF.NX_ = CornerA.x();
						VertexF.NY_ = CornerA.y();
						VertexF.NZ_ = 0.0f;
						VertexF.U_ = UVBounds.x();
						VertexF.V_ = UVBounds.y();
						VertexF.RGBA_ = Colour;
					}
				}

				BcAssert( NoofParticlesRendered == NoofParticlesToRender );
				UploadFence_.decrement();
			} );
	}

	// Update uniform buffer.
	if( IsLocalSpace_ )
	{
		ObjectUniforms_.WorldTransform_ = getParentEntity()->getWorldMatrix();
	}
	else
	{
		ObjectUniforms_.WorldTransform_ = Transform_;
	}

	// Upload uniforms.
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

	// Draw particles last.
	if( NoofParticlesToRender > 0 )
	{
		auto* ViewRenderData = static_cast< ScnParticleSystemViewRenderData* >( RenderContext.ViewRenderData_ );
		RsRenderSort Sort = RenderContext.Sort_;
		Sort.Layer_ = 15;

		// Add to frame.
		RenderContext.pFrame_->queueRenderNode( Sort,
			[
				GeometryBinding = GeometryBinding_.get(),
				ProgramBinding = ViewRenderData->ProgramBinding_.get(),
				RenderState = ViewRenderData->RenderState_,
				FrameBuffer = RenderContext.pViewComponent_->getFrameBuffer(),
				Viewport = RenderContext.pViewComponent_->getViewport(),
				NoofParticlesToRender 
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
					RsTopologyType::TRIANGLE_LIST, 0, NoofParticlesToRender * 6,
					0, 1 );
			} );
	}
}

//////////////////////////////////////////////////////////////////////////
// createViewRenderData
//virtual
class ScnViewRenderData* ScnParticleSystemComponent::createViewRenderData( class ScnViewComponent* View )
{
	ScnParticleSystemViewRenderData* ViewRenderData = new ScnParticleSystemViewRenderData();

	ScnShaderPermutationFlags ShaderPermutation = ScnShaderPermutationFlags::MESH_PARTICLE_3D;
	ShaderPermutation |= View->getRenderPermutation();
	auto Program = Material_->getProgram( ShaderPermutation );
	auto ProgramBindingDesc = Material_->getProgramBinding( ShaderPermutation );
	{
		auto Slot = Program->findUniformBufferSlot( "ScnShaderObjectUniformBlockData" );
		if( Slot != BcErrorCode )
		{	
			ProgramBindingDesc.setUniformBuffer( Slot, UniformBuffer_.get(), 0, sizeof( ScnShaderObjectUniformBlockData ) );
		}
	}
	{
		auto Slot = Program->findUniformBufferSlot( "ScnShaderViewUniformBlockData" );
		if( Slot != BcErrorCode )
		{	
			ProgramBindingDesc.setUniformBuffer( Slot, View->getViewUniformBuffer(), 0, sizeof( ScnShaderViewUniformBlockData ) );
		}
	}
	ViewRenderData->ProgramBinding_ = RsCore::pImpl()->createProgramBinding( Program, ProgramBindingDesc, getFullName().c_str() );
	ViewRenderData->RenderState_ = Material_->getRenderState();

	ViewRenderData->setSortPassType( getSortPassType( View ) );
	return ViewRenderData;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnParticleSystemComponent::onAttach( ScnEntityWeakRef Parent )
{
	// TODO: Allow different types of geom for this.
	// TODO: Use index buffer.
	// Calc what we need.
	BcU32 NoofVertices = NoofParticles_ * 6;	// 2x3 tris.

	// Create vertex declaration.
	VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration( 
		RsVertexDeclarationDesc( 4 )
			.addElement( RsVertexElement( 0, 0,				4,		RsVertexDataType::FLOAT32,		RsVertexUsage::POSITION,	0 ) )
			.addElement( RsVertexElement( 0, 16,			4,		RsVertexDataType::FLOAT32,		RsVertexUsage::TANGENT,		0 ) )
			.addElement( RsVertexElement( 0, 32,			2,		RsVertexDataType::FLOAT32,		RsVertexUsage::TEXCOORD,	0 ) )
			.addElement( RsVertexElement( 0, 40,			4,		RsVertexDataType::UBYTE_NORM,	RsVertexUsage::COLOUR,		0 ) ),
		getFullName().c_str() );

	// Allocate vertex buffer.
	VertexBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc( 
			RsResourceBindFlags::VERTEX_BUFFER, 
			RsResourceCreationFlags::STREAM, 
			NoofVertices * sizeof( ScnParticleVertex ) ),
		getFullName().c_str() );

	RsGeometryBindingDesc GeometryBindingDesc;
	GeometryBindingDesc.setVertexDeclaration( VertexDeclaration_.get() );
	GeometryBindingDesc.setVertexBuffer( 0, VertexBuffer_.get(), sizeof( ScnParticleVertex ) );
	GeometryBinding_ = RsCore::pImpl()->createGeometryBinding( GeometryBindingDesc, getFullName().c_str() );

	UniformBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc( 
			RsResourceBindFlags::UNIFORM_BUFFER,
			RsResourceCreationFlags::STREAM,
			sizeof( ObjectUniforms_ ) ),
		getFullName().c_str() );

	// Allocate particles.
	pParticleBuffer_ = new ScnParticle[ NoofParticles_ ];
	BcMemZero( pParticleBuffer_, sizeof( ScnParticle ) * NoofParticles_ );

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnParticleSystemComponent::onDetach( ScnEntityWeakRef Parent )
{
	UploadFence_.wait();

	delete [] pParticleBuffer_;

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
//static
void ScnParticleSystemComponent::updateParticles( const ScnComponentList& Components )
{
	BcF32 Tick = SysKernel::pImpl()->getFrameTime();
	
	for( auto Component : Components )
	{
		BcAssert( Component->isTypeOf< ScnParticleSystemComponent >() );
		auto* ParticleSystemComponent = static_cast< ScnParticleSystemComponent* >( Component.get() );

		// TODO: Iterate over every "affector" at a time, rather than by particle.
		// - See "updateParticle".

		MaAABB FullAABB( MaVec3d( 0.0f, 0.0f, 0.0f ), MaVec3d( 0.0f, 0.0f, 0.0f ) );

		// Not optimal, but clear code is clear. (For now...)
		for( BcU32 Idx = 0; Idx < ParticleSystemComponent->NoofParticles_; ++Idx )
		{
			ScnParticle& Particle = ParticleSystemComponent->pParticleBuffer_[ Idx ];

			if( Particle.Alive_ )
			{
				// Update particle.
				ParticleSystemComponent->updateParticle( Particle, Tick );

				// Expand AABB by particle's max bounds.
				const BcF32 MaxHalfSize = BcMax( Particle.Scale_.x(), Particle.Scale_.y() ) * 0.5f;
				FullAABB.expandBy( Particle.Position_ - MaVec3d( MaxHalfSize, MaxHalfSize, MaxHalfSize ) );
				FullAABB.expandBy( Particle.Position_ + MaVec3d( MaxHalfSize, MaxHalfSize, MaxHalfSize ) );
			}
		}

		// Transform AABB.
		if( ParticleSystemComponent->IsLocalSpace_ )
		{
			const MaMat4d& WorldTransform = ParticleSystemComponent->getParentEntity()->getWorldMatrix();
			ParticleSystemComponent->AABB_ = FullAABB.transform( WorldTransform );
		}
		else
		{
			ParticleSystemComponent->AABB_ = FullAABB;
		}
	}
}
