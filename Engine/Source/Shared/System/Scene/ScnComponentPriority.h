#pragma once

//////////////////////////////////////////////////////////////////////////
// ScnComponentPriority
enum class ScnComponentPriority : BcS32
{
	DEFAULT_PRE_UPDATE = -10000,

	DEBUG_RENDER_CLEAR,
	CANVAS_CLEAR,

	ANIMATION_DECODE,
	PHYSICS_WORLD_SIMULATE, 
	PHYSICS_RIGID_BODY_UPDATE,

	ENTITY_UPDATE,

	ANIMATION_POSE,

	// update

	
	DEFAULT_UPDATE = 0,


	// postUpdate
	DEFAULT_POST_UPDATE = 10000,

	MODEL_UPDATE,
	PARTICLE_SYSTEM_UPDATE,
	TILEMAP_UPDATE,
	SPRITE_UPDATE,
	SOUND_LISTENER_UPDATE,
	SOUND_EMITTER_UPDATE,
	SPATIAL_UPDATE,

	ANIMATION_ADVANCE,

	// debugDraw

	DEFAULT_DEBUG_DRAW = 20000,
	PHYSICS_WORLD_DEBUG_DRAW,

	VIEW_UPDATE,

	// render
	UPDATE_ENVIRONMENT_PROBES,
	VIEW_RENDER,
};

inline ScnComponentPriority operator + ( ScnComponentPriority A, ScnComponentPriority B )
{
	return (ScnComponentPriority)( (BcS32)A + (BcS32)B );
}

inline ScnComponentPriority operator + ( ScnComponentPriority A, BcS32 B )
{
	return (ScnComponentPriority)( (BcS32)A + B );
}

inline ScnComponentPriority operator + ( BcS32 A, ScnComponentPriority B )
{
	return (ScnComponentPriority)( (BcS32)A + B );
}

inline ScnComponentPriority operator - ( ScnComponentPriority A, ScnComponentPriority B )
{
	return (ScnComponentPriority)( (BcS32)A - (BcS32)B );
}

inline ScnComponentPriority operator - ( ScnComponentPriority A, BcS32 B )
{
	return (ScnComponentPriority)( (BcS32)A - B );
}

inline ScnComponentPriority operator - ( BcS32 A, ScnComponentPriority B )
{
	return (ScnComponentPriority)( (BcS32)A - B );
}
