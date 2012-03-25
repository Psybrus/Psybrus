/**************************************************************************
*
* File:		GaGameUnit.h
* Author:	Neil Richardson 
* Ver/Date:	24/03/12
* Description:
*		Game unit.
*		
*
*
* 
**************************************************************************/

#ifndef __GAGAMEUNIT_H__
#define __GAGAMEUNIT_H__

#include "Psybrus.h"

#include "GaEvents.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class GaGameSimulator;

//////////////////////////////////////////////////////////////////////////
// GaGameUnitIDList
typedef std::vector< BcU32 > GaGameUnitIDList;

//////////////////////////////////////////////////////////////////////////
// GaGameUnitState
struct GaGameUnitState
{
	BcFixedVec2d Position_;
	BcFixedVec2d Velocity_;
};

//////////////////////////////////////////////////////////////////////////
// GaGameUnitDescriptor
struct GaGameUnitDescriptor
{
	BcU32 Type_;							// Type.
	BcFixedVec2d Size_;						// Unit size.
	BcFixed MoveSpeed_;						// How many units per second it can move.
	BcFixed RateOfAttack_;					// Rate that we can attack.
	BcFixed Range_;							// Range of attack.
	BcFixed MinRange_;						// Min range.
	BcFixed Health_;						// Unit health.
	BcBool Armoured_;						// Is armoured?
	GaGameUnitDescriptor* pDamageUnit_;		// Unit to use for damage.
};

//////////////////////////////////////////////////////////////////////////
// GaGameUnit
class GaGameUnit
{
public:
	GaGameUnit( GaGameSimulator* pSimulator, const GaGameUnitDescriptor& Desc, BcU32 TeamID, BcU32 ID, const BcFixedVec2d& Position );
	~GaGameUnit();

	BcU32 getTeamID() const;
	BcU32 getID() const;
	const BcFixedVec2d& getPosition() const;
	const BcFixedVec2d& getVelocity() const;
	BcBool isDead() const;

	BcFixedVec2d getInterpolatedPosition( BcFixed Fraction ) const;

	const GaGameUnitDescriptor& getDesc() const;
	BcFixed getMaxHealth() const;
	BcFixed getHealth() const;
	BcFixed getMaxAttackTime() const;
	BcFixed getAttackTime() const;

	BcU32 getChecksum() const;

	void setBehaviourIdle();
	void setBehaviourGuard();
	void setBehaviourMove( const BcFixedVec2d& Target );
	void setBehaviourAttack( BcU32 TargetUnitID );
	void setBehaviourDamage( const BcFixedVec2d& Target );
	void setBehaviourDead();

	BcBool inRangeForAttack( BcU32 TargetID );
	void doAttack( BcU32 TargetID );

	void applyDamage( BcFixed Amount );

	void tick( BcFixed Delta );
	void tickState( BcFixed Delta );
	void tickBehaviour( BcFixed Delta );
	void advanceState();	

	void render( ScnCanvasComponentRef Canvas, BcFixed TimeFraction );
	void renderShadow( ScnCanvasComponentRef Canvas, BcFixed TimeFraction );
	void renderHUD( ScnCanvasComponentRef Canvas, BcFixed TimeFraction );
	void renderSelectionHUD( ScnCanvasComponentRef Canvas, BcFixed TimeFraction );

private:
	enum TBehaviour
	{
		BEHAVIOUR_IDLE = 0,
		BEHAVIOUR_GUARD,
		BEHAVIOUR_MOVE,
		BEHAVIOUR_ATTACK,
		BEHAVIOUR_DAMAGE,
		BEHAVIOUR_DEAD
	};

	GaGameSimulator* pSimulator_;
	GaGameUnitDescriptor Desc_;
	BcU32 TeamID_;
	BcU32 ID_;
	TBehaviour Behaviour_;
	GaGameUnitState NextState_;
	GaGameUnitState CurrState_;
	GaGameUnitState PrevState_;

	BcFixed AttackTimer_;
	BcFixed Health_;
	
	BcFixedVec2d MoveTargetPosition_;
};

#endif
