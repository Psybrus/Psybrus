/**************************************************************************
*
* File:		Rendering/ScnParticleSystemComponent.h
* Author:	Neil Richardson 
* Ver/Date:	19/04/12
* Description:
*		Particle system component.
*		Renders and processes particle system.
*
*
* 
**************************************************************************/

#ifndef __SCNPARTICLESYSTEMCOMPONENT_H__
#define __SCNPARTICLESYSTEMCOMPONENT_H__

#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef ReObjectRef< class ScnParticleSystemComponent > ScnParticleSystemComponentRef;
typedef ReObjectRef< class ScnParticleSystemComponent, false > ScnParticleSystemComponentWeakRef;

//////////////////////////////////////////////////////////////////////////
// Typedefs
struct ScnParticleVertex
{
	BcF32 X_, Y_, Z_, W_;
	BcF32 NX_, NY_, NZ_, NW_;			// Offset.
	BcF32 U_, V_;
	BcU32 RGBA_;
};

//////////////////////////////////////////////////////////////////////////
// ScnParticle
struct ScnParticle					// TODO: Factor our into affectors so we can store minimum amount.
{
	MaVec3d Position_;				// Position.
	MaVec3d Velocity_;				// Velocity.
	MaVec3d Acceleration_;			// Acceleration.

	MaVec2d Scale_;					// Scale.
	MaVec2d MinScale_;				// Min scale. (time based)
	MaVec2d MaxScale_;				// Max scale. (time based)

	BcF32 Rotation_;
	BcF32 RotationMultiplier_;		// Rotation mult.

	RsColour Colour_;				// Colour;
	RsColour MinColour_;			// Min colour. (time based)
	RsColour MaxColour_;			// Max colour. (time based)

	BcU32 TextureIndex_;			// Texture index.
	BcF32 CurrentTime_;			// Current time.
	BcF32 MaxTime_;				// Max time.
	BcBool Alive_;					// Are we alive?
};

//////////////////////////////////////////////////////////////////////////
// ScnParticleSystemComponent
class ScnParticleSystemComponent:
	public ScnRenderableComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnParticleSystemComponent, ScnRenderableComponent );

	ScnParticleSystemComponent();
	virtual ~ScnParticleSystemComponent();

public:
	void initialise() override;

	MaAABB getAABB() const override;

	void render( ScnRenderContext & RenderContext ) override;
	class ScnViewRenderData* createViewRenderData( class ScnViewComponent* View ) override;
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	BcBool allocParticle( ScnParticle*& pParticle );

	/**
	 * Set transform if not local space.
	 */
	void setTransform( MaMat4d Transform ){ Transform_ = Transform; }

private:
	void updateParticle( ScnParticle& Particle, BcF32 Tick );

	static void updateParticles( const ScnComponentList& Components );

private:
	// Graphics data.
	RsVertexDeclarationUPtr VertexDeclaration_;
	RsBufferUPtr VertexBuffer_;
	RsBufferUPtr UniformBuffer_;
	RsGeometryBindingUPtr GeometryBinding_;
	ScnShaderObjectUniformBlockData	ObjectUniforms_;

	// Particle data.
	ScnParticle* pParticleBuffer_;
	BcU32 NoofParticles_;
	BcU32 PotentialFreeParticle_;
	
	// 
	ScnMaterialRef Material_;
	BcU32 WorldTransformParam_;

	//
	bool IsLocalSpace_;

	// UV bounds.
	std::vector< MaVec4d > UVBounds_;

	// Transform.
	MaMat4d Transform_;

	// AABB
	MaAABB AABB_;

	// Fences for render.
	SysFence UploadFence_;
};

#endif

