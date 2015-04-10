#pragma once

//////////////////////////////////////////////////////////////////////////
// ScnComponentPriority
enum class ScnComponentPriority : BcS32
{
	DEFAULT_PRE_UPDATE = -10000

	DEBUG_RENDER_CLEAR,
	CANVAS_CLEAR,

	LIGHT_UPDATE,

	ANIMATION_DECODE,

	// update

	ANIMATION_POSE,
	PHYSICS_WORLD_SIMULATE,
	PHYSICS_RIGID_BODY_UPDATE
	
	DEFAULT_UPDATE = 0,


	// postUpdate
	DEFAULT_POST_UPDATE = 10000,

	MODEL_UPDATE,
	PARTICLE_SYSTEM_UPDATE,
	SPRITE_UPDATE,
	SOUND_LISTENER_UPDATE,

	VIEW_UPDATE,

	// render
	VIEW_RENDER,
};