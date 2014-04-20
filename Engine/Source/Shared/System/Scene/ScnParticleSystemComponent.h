/**************************************************************************
*
* File:		ScnParticleSystemComponent.h
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

#include "System/Scene/ScnRenderableComponent.h"
#include "System/Scene/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef ReObjectRef< class ScnParticleSystemComponent > ScnParticleSystemComponentRef;
typedef ReObjectRef< class ScnParticleSystemComponent, false > ScnParticleSystemComponentWeakRef;

//////////////////////////////////////////////////////////////////////////
// Typedefs
struct ScnParticleVertex
{
	BcF32 X_, Y_, Z_;
	BcF32 NX_, NY_, NZ_;			// Offset.
	BcF32 U_, V_;
	BcU32 RGBA_;
};

//////////////////////////////////////////////////////////////////////////
// ScnParticle
struct ScnParticle					// TODO: Factor our into affectors so we can store minimum amount.
{
	BcVec3d Position_;				// Position.
	BcVec3d Velocity_;				// Velocity.
	BcVec3d Acceleration_;			// Acceleration.

	BcVec2d Scale_;					// Scale.
	BcVec2d MinScale_;				// Min scale. (time based)
	BcVec2d MaxScale_;				// Max scale. (time based)

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
	DECLARE_RESOURCE( ScnRenderableComponent, ScnParticleSystemComponent );

public:
	virtual void						initialise( const Json::Value& Object );
	virtual void						create();
	virtual void						destroy();

	virtual BcAABB						getAABB() const;

	virtual void						postUpdate( BcF32 Tick );
	virtual void						render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	ScnMaterialComponentRef				getMaterialComponent();

	BcBool								allocParticle( ScnParticle*& pParticle );

private:
	void								updateParticle( ScnParticle& Particle, BcF32 Tick );
	void								updateParticles( BcF32 Tick );

private:
	struct TVertexBuffer
	{
		ScnParticleVertex*				pVertexArray_;
		RsVertexBuffer*					pVertexBuffer_;
		RsPrimitive*					pPrimitive_;
	};

	// Graphics data.
	TVertexBuffer						VertexBuffers_[ 2 ];
	BcU32								CurrentVertexBuffer_;

	// Particle data.
	ScnParticle*						pParticleBuffer_;
	BcU32								NoofParticles_;
	BcU32								PotentialFreeParticle_;
	
	// 
	ScnMaterialComponentRef				MaterialComponent_;
	BcU32								WorldTransformParam_;

	//
	BcBool								IsLocalSpace_;

	// UV bounds.
	std::vector< BcVec4d >				UVBounds_;

	SysFence							UpdateFence_;

	BcAABB								AABB_;
};

#endif

