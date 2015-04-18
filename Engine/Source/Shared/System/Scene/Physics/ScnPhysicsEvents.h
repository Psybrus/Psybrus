#pragma once

#include "Events/EvtEvent.h"

////////////////////////////////////////////////////////////////////////////////
// Event enum
enum class ScnPhysicsEvents : EvtID
{
	FIRST = EVT_MAKE_ID( 'S', 'p', 0 ),

	COLLISION,
	
	//
	MAX
};

////////////////////////////////////////////////////////////////////////////////
// ScnPhysicsEventCollision
struct ScnPhysicsEventCollision: EvtEvent< ScnPhysicsEventCollision >
{
	class ScnPhysicsRigidBodyComponent* BodyA_;	
	class ScnPhysicsRigidBodyComponent* BodyB_;
	struct ContactPoint
	{
		MaVec3d PointA_;
		MaVec3d PointB_;
	};
	BcU32 NoofContactPoints_;
	std::array< ContactPoint, 8 > ContactPoints_;
};
